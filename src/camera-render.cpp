#include "camera.h"
#include "triangle.h"
#include "fog.h"
#include <imgui.h>
#include <functional>
#include <SFML/System/Clock.hpp>

struct TransparentTriangle{
    float z;
    Triangle tri;
};

sf::Clock performanceClock;

void Camera::render(RenderTarget *frame) {
    for (size_t i = 0; i < frame->size.x*frame->size.y; i++) {
        frame->framebuffer[i] = Color{0, 0, 0, 1};
        frame->zBuffer[i]=INFINITY;
    }

    makePerspectiveProjectionMatrix();

#pragma region // ===== PROJECT VERTICES & BUILD TRIANGLES =====

    std::vector<Triangle> triangles;
    std::vector<TransparentTriangle> transparents;

    std::function<void(Object*)> handleObject = [&](Object *obj) {
        for (auto &&comp : obj->components) {
            if(MeshComponent *meshComp = dynamic_cast<MeshComponent*>(comp)) {
                Mesh *mesh = meshComp->mesh;
                Projection projectedVertices[mesh->n_vertices];

                for (size_t j = 0; j < mesh->n_vertices; j++) {
                    Vertex vV = mesh->vertices[j];

                    projectedVertices[j] = perspectiveProject(vV.position * obj->transform);
                    auto normal = (vV.normal * obj->transformRotation).normalized();
                    projectedVertices[j].normal = normal;
                }

                for (size_t j = 0; j < mesh->n_faces; j++) {
                    Face face = mesh->faces[j];
                    Projection v1s = projectedVertices[face.v1],
                               v2s = projectedVertices[face.v2],
                               v3s = projectedVertices[face.v3];
                    Vector3f normalS = (v3s.screenPos - v1s.screenPos).cross(v2s.screenPos - v1s.screenPos).normalized();

                    Triangle tri = {
                        .s1 = v1s,
                        .s2 = v2s,
                        .s3 = v3s,
                        .uv1 = mesh->vertices[face.v1].uv,
                        .uv2 = mesh->vertices[face.v2].uv,
                        .uv3 = mesh->vertices[face.v3].uv,
                        .mat = face.material,
                        .cull = normalS.z < 0
                    };
                    if(face.material->flags & MaterialFlags::Transparent) {
                        transparents.push_back(TransparentTriangle{(v1s.screenPos.z + v2s.screenPos.z + v3s.screenPos.z) / 3, tri});
                    }
                    else {
                        triangles.push_back(tri);
                    }
                }
            }
        }
        for (auto &&child : obj->children)
            handleObject(child);
    };

    for (auto &&obj : scene->objects)
        handleObject(obj);

#pragma endregion


#pragma region // ===== DRAW TRIANGLES =====

    performanceClock.restart();

    for (auto &&tri : triangles)
        drawTriangle(frame, tri, frame->deferred);

    geometryTime = performanceClock.restart().asMilliseconds();

    // Deferred pass
    if(frame->deferred) {
        for (size_t i = 0; i < frame->size.x*frame->size.y; i++)
        {
            if(frame->zBuffer[i] == INFINITY) // No fragment here
                continue;
            Fragment &f = frame->gBuffer[i];
            if (frame->deferred && !(f.mat->flags & MaterialFlags::AlphaCutout))
                f.baseColor = f.mat->getBaseColor(f.uv, f.dUVdx, f.dUVdy);
            frame->framebuffer[i] = f.mat->shade(f, frame->framebuffer[i]);
        }
    }

    lightingTime = performanceClock.restart().asMilliseconds();

    auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
    std::sort(transparents.begin(), transparents.end(), compareZ);
    for (auto &&tri : transparents)
        drawTriangle(frame, tri.tri, false);
    
    forwardTime = performanceClock.restart().asMilliseconds();
    performanceClock.stop();

#pragma endregion

    if(scene->fogColor.a > 0)
        fog();
}
