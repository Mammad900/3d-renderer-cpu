#ifndef __RENDER_H__
#define __RENDER_H__
#include "triangle.h"
#include "object.h"
#include "color.h"
#include "data.h"
#include "matrix.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using sf::Vector3f, sf::Vector2f;
using std::max;

struct TransparentTriangle{
    float z;
    Triangle *tri;
};

void fog();

void render(Scene *scene, RenderTarget *frame) {
    for (size_t i = 0; i < frame->size.x*frame->size.y; i++)
    {
        frame->framebuffer[i] = Color{0, 0, 0, 1};
        frame->zBuffer[i]=INFINITY;
    }

#pragma region // ===== COUNT VERTICES & FACES =====

    int total_vertices = 0;
    int total_faces = 0;
    for (auto &&obj : scene->objects) {
        obj->update();
        for (auto &&comp : obj->components)
        if(MeshComponent *meshComp = dynamic_cast<MeshComponent*>(comp)) {
            Mesh *mesh = meshComp->mesh;
            total_vertices += mesh->n_vertices;
            total_faces += mesh->n_faces;
        }
    }
    
#pragma endregion

    makePerspectiveProjectionMatrix();

    scene->camDirection = rotate({0, 0, 1}, scene->camRotation);

    for (size_t i = 0; i < scene->lights.size(); i++) {
        Light &light = scene->lights[i];
        if(!light.isPointLight)
            light.direction = rotate(Vector3f{0, -1, 0}, light.rotation);
    }

#pragma region // ===== PROJECT VERTICES & BUILD TRIANGLES =====

    std::vector<Triangle> triangles(total_faces);
    std::vector<TransparentTriangle> transparents;
    int triI = 0;
    for (auto &&obj : scene->objects)
        for (auto &&comp : obj->components)
            if(MeshComponent *meshComp = dynamic_cast<MeshComponent*>(comp)) {
                Mesh *mesh = meshComp->mesh;
                Projection projectedVertices[mesh->n_vertices];

                for (size_t j = 0; j < mesh->n_vertices; j++) {
                    Vertex vV = mesh->vertices[j];

                    projectedVertices[j] = perspectiveProject(vV.position * obj->myTransform);
                    projectedVertices[j].normal = (vV.normal * obj->myRotation).normalized(); 
                    // ^ The transform we applied to the normal vector also scales and translates it, have to undo those.
                }

                for (size_t j = 0; j < mesh->n_faces; j++) {
                    Face face = mesh->faces[j];
                    Projection v1s = projectedVertices[face.v1],
                            v2s = projectedVertices[face.v2],
                            v3s = projectedVertices[face.v3];
                    Vector3f normalS = (v3s.screenPos - v1s.screenPos).cross(v2s.screenPos - v1s.screenPos).normalized();

                    triangles[triI] = Triangle{
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
                        transparents.push_back(TransparentTriangle{(v1s.screenPos.z + v2s.screenPos.z + v3s.screenPos.z) / 3, &triangles[triI]});
                    }
                    triI++;
                }
            }
    

#pragma endregion

#pragma region // ===== DRAW TRIANGLES =====

    for (int i = 0; i < total_faces; i++) {
        if(triangles[1].mat->flags & MaterialFlags::Transparent) continue;
        drawTriangle(frame, triangles[i], frame->deferred);
    }

    // Deferred pass
    if(frame->deferred) {
        for (size_t i = 0; i < frame->size.x*frame->size.y; i++)
        {
            if(frame->zBuffer[i] == INFINITY) // No fragment here
                continue;
            Fragment &f = frame->gBuffer[i];
            frame->framebuffer[i] = f.mat->shade(f, frame->framebuffer[i]);
        }
    }

    auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
    std::sort(transparents.begin(), transparents.end(), compareZ);
    for (auto &&tri : transparents) {
        drawTriangle(frame, *tri.tri, false);
    }
    

#pragma endregion

#pragma region // ===== FOG =====
    if(scene->fogColor.a > 0)
        fog();

#pragma endregion
}

void fog() {

    float tanFOV = tan(scene->fov * M_PI / 360);
    for (int y = 0; y < (int)frame->size.y; y++) {
        for (int x = 0; x < (int)frame->size.x; x++) {
            size_t i = frameBufferIndex({x, y});
            float z = frame->zBuffer[i];
            Vector2f world = Vector2f{x / (float)frame->size.x, y / (float)frame->size.y} * z * tanFOV; // Reconstruct world space X and Y
            float dist = std::sqrt(
                world.lengthSquared() + z * z
            ); // Account for X and Y, to make it radial vs flat
            float visibility = std::clamp(
                std::powf(0.5f, dist * scene->fogColor.a), 0.0f, 1.0f
            ); // Exponential falloff
            Color result = frame->framebuffer[i] * visibility +
                           scene->fogColor * (1 - visibility); // Lerp
            frame->framebuffer[i] = result;
        }
    }
}

#endif /* __RENDER_H__ */
