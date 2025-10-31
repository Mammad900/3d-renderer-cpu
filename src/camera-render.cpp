#include "camera.h"
#include "color.h"
#include "data.h"
#include "texture.h"
#include "triangle.h"
#include "fog.h"
#include "multithreading.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include <functional>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <typeindex>

void Camera::render() {
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene) return;

    if(shadowMap) {
        makePerspectiveProjectionMatrix();

        std::fill(tFrame->zBuffer.begin(), tFrame->zBuffer.end(), INFINITY);

        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;

        buildTriangles(transparents, triangles);

        for (auto &&tri : triangles)
            drawTriangle(this, tri, tFrame->deferred);
    } 
    else {
        timing.clock.restart();
        makePerspectiveProjectionMatrix();
        std::fill(tFrame->zBuffer.begin(), tFrame->zBuffer.end(), INFINITY);
        if(!tFrame->deferred)
            drawSkyBox();
        timing.skyBoxTime.push(timing.clock);


        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;
        buildTriangles(transparents, triangles);

        timing.renderPrepareTime.push(timing.clock);


        for (auto &&tri : triangles)
            drawTriangle(this, tri, tFrame->deferred);

        timing.geometryTime.push(timing.clock);


        // Deferred pass
        if(tFrame->deferred)
            startThreads(this, false);

        timing.lightingTime.push(timing.clock);


        auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
        std::sort(transparents.begin(), transparents.end(), compareZ);
        for (auto &&tri : transparents)
            drawTriangle(this, tri.tri, false);
        
        timing.forwardTime.push(timing.clock);
        timing.clock.stop();


        if(scene->volume)
            startThreads(this, true); // Even if deferred rendering is disabled, this can be multithreaded
    }
}

void Camera::buildTriangles(
    std::vector<TransparentTriangle> &transparents,
    std::vector<Triangle> &triangles
) {
    std::function<void(shared_ptr<Object>)> handleObject = [&](shared_ptr<Object> obj) {
        for (auto &&comp : obj->components) {
            if (MeshComponent *meshComp = dynamic_cast<MeshComponent *>(comp.get())) {
                shared_ptr<Mesh> mesh = meshComp->mesh;
                Projection projectedVertices[mesh->vertices.size()];

                for (size_t j = 0; j < mesh->vertices.size(); j++) {
                    Vertex vV = mesh->vertices[j];

                    projectedVertices[j] = perspectiveProject(vV.position * obj->transform);
                    auto normal = (vV.normal * obj->transformNormals).normalized();
                    projectedVertices[j].normal = normal;
                }

                for (size_t j = 0; j < mesh->faces.size(); j++) {
                    Face &face = mesh->faces[j];
                    Projection v1s = projectedVertices[face.v1],
                               v2s = projectedVertices[face.v2],
                               v3s = projectedVertices[face.v3];
                    Vec3 normalS = (v3s.screenPos - v1s.screenPos).cross(v2s.screenPos - v1s.screenPos).normalized();

                    Triangle tri = {
                        .s1 = v1s,
                        .s2 = v2s,
                        .s3 = v3s,
                        .uv1 = mesh->vertices[face.v1].uv,
                        .uv2 = mesh->vertices[face.v2].uv,
                        .uv3 = mesh->vertices[face.v3].uv,
                        .mat = face.material,
                        .face = &face,
                        .mesh = mesh,
                        .cull = normalS.z < 0
                    };
                    if (face.material->flags.transparent) {
                        transparents.push_back(TransparentTriangle{
                            (v1s.screenPos.z + v2s.screenPos.z + v3s.screenPos.z) / 3, tri });
                    } else {
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
}

void skyBoxPixel(Camera *camera, RenderTarget *frame, uint i, uint x, uint y) {
    Vec3 lookVector = camera->screenSpaceToCameraSpace(x, y, 1) * camera->obj->transformRotation;
    lookVector = lookVector.normalized();

    Vector2f uv {
        0.5f +(atan2f(lookVector.z, lookVector.x) / (2.0f * M_PIf)),
        0.5f - (asinf(lookVector.y) / M_PIf)
    };

    frame->framebuffer[i] = scene->skyBox->sample(uv, {0, 0}, {0, 0});
}

SolidTexture<Color> *checkSolidSkyBox(shared_ptr<Texture<Color>> skyBox) {
    // dynamic_cast alone doesn't work because we don't want derived classes
    std::type_index ti(typeid(*scene->skyBox));
    if (ti == std::type_index(typeid(SolidTexture<Color>)))
        return dynamic_cast<SolidTexture<Color> *>(scene->skyBox.get());
    return nullptr;
}

void Camera::drawSkyBox() {
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene) return;

    if(auto solid = checkSolidSkyBox(scene->skyBox)) {
        std::fill(tFrame->framebuffer.begin(), tFrame->framebuffer.end(), solid->value);
        return;
    }

    for (uint y = 0; y < tFrame->size.y; y++) {
        for (uint x = 0; x < tFrame->size.x; x++) {
            size_t i = y * tFrame->size.x + x;
            skyBoxPixel(this, tFrame, i, x, y);
        }
    }
}


void deferredPass(uint n, uint i0, Camera *camera) {
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    if(!scene) return;

    RenderTarget *frame = camera->tFrame;

    SolidTexture<Color> *solidSkyBox = checkSolidSkyBox(scene->skyBox);

    for (size_t i = i0; i < frame->size.x * frame->size.y; i += n) {
        if (frame->zBuffer[i] == INFINITY) { // No fragment here, must be skyBox
            if (solidSkyBox) {
                frame->framebuffer[i] = solidSkyBox->value; // No need to compute UV
            } else {
                int x = i % frame->size.x, y= i / frame->size.x;
                skyBoxPixel(camera, frame, i, x, y);
            }
            continue;
        }

        Fragment &f = frame->gBuffer[i];
        if (frame->deferred && !f.face->material->flags.alphaCutout)
            f.baseColor = f.face->material->getBaseColor(f.uv, f.dUVdx, f.dUVdy);
        frame->framebuffer[i] = f.face->material->shade(f, frame->framebuffer[i], *scene);
    }
}

void fogPass(uint n, uint i0, Camera *camera) {
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    if(!scene) return;
    if(!scene->volume)
        return;

    RenderTarget *frame = camera->tFrame;
    for (size_t i = i0; i < frame->size.x * frame->size.y; i += n) {
        if(frame->zBuffer[i] == INFINITY && !scene->godRays) // Sky-box pixels don't get fog unless its godRays
            continue;
        int x = i % frame->size.x, y= i / frame->size.x;
        float z = camera->tFrame->zBuffer[i];

        if(z == INFINITY) {
            if(scene->godRays)
                z = camera->farClip;
            else
                return;
        }

        camera->tFrame->framebuffer[i] = sampleFog(
            camera->screenSpaceToWorldSpace(x, y, z), 
            camera->obj->globalPosition,
            camera->tFrame->framebuffer[i],
            *scene,
            scene->volume
        );
    }
}
