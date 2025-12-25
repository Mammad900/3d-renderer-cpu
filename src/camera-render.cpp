#include "camera.h"
#include "color.h"
#include "data.h"
#include "environmentMap.h"
#include "object.h"
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

    maximumColor = 0;

    if(shadowMap) {
        makePerspectiveProjectionMatrix();

        std::fill(frame->zBuffer.begin(), frame->zBuffer.end(), INFINITY);

        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;

        buildTriangles(transparents, triangles);

        for (auto &&tri : triangles)
            drawTriangle(this, tri, frame->deferred);
    } 
    else {
        timing.clock.restart();

        makePerspectiveProjectionMatrix();
        std::fill(frame->zBuffer.begin(), frame->zBuffer.end(), INFINITY);
        if(frame->deferred) {
            for (Fragment &f : frame->gBuffer)
                f.z = INFINITY;
            std::fill(frame->transparencyHeads.begin(), frame->transparencyHeads.end(), (uint32_t)-1);
            frame->transparencyFragments.clear();
        }
        else
            drawSkyBox();

        timing.skyBoxTime.push(timing.clock);


        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;
        buildTriangles(transparents, triangles);

        timing.renderPrepareTime.push(timing.clock);


        for (auto &&tri : triangles)
            drawTriangle(this, tri, frame->deferred);

        if(frame->deferred)
            for (auto &&tri : transparents)
                drawTriangle(this, tri.tri, true);

        timing.geometryTime.push(timing.clock);


        // Deferred pass
        if(frame->deferred)
            startThreads(this, false);

        timing.lightingTime.push(timing.clock);

        if(!frame->deferred) {
            auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
            std::sort(transparents.begin(), transparents.end(), compareZ);
            for (auto &&tri : transparents)
                drawTriangle(this, tri.tri, false);
        }
        
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
    shared_ptr<Scene> scene = obj->scene.lock();
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
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    Vec3 lookVector = camera->screenSpaceToCameraSpace(x, y, 1) * camera->obj->transformRotation;
    lookVector = lookVector.normalized();
    frame->framebuffer[i] = scene->skyBox->sample(lookVector);
}

SolidEnvironmentMap *checkSolidSkyBox(shared_ptr<EnvironmentMap> skyBox) {
    // dynamic_cast alone doesn't work because we don't want derived classes
    std::type_index ti(typeid(*skyBox));
    if (ti == std::type_index(typeid(SolidEnvironmentMap)))
        return dynamic_cast<SolidEnvironmentMap *>(skyBox.get());
    return nullptr;
}

void Camera::drawSkyBox() {
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene) return;

    if(auto solid = checkSolidSkyBox(scene->skyBox)) {
        std::fill(frame->framebuffer.begin(), frame->framebuffer.end(), solid->value);
        return;
    }

    for (uint y = 0; y < frame->size.y; y++) {
        for (uint x = 0; x < frame->size.x; x++) {
            size_t i = y * frame->size.x + x;
            skyBoxPixel(this, frame, i, x, y);
        }
    }
}


void deferredPass(uint n, uint i0, Camera *camera) {
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    if(!scene) return;

    RenderTarget *frame = camera->frame;

    SolidEnvironmentMap *solidSkyBox = checkSolidSkyBox(scene->skyBox);

    for (size_t i = i0; i < frame->size.x * frame->size.y; i += n) {
        Fragment &f = frame->gBuffer[i];
        float z = f.z; // keep track of last shaded Z for fog
        if (z == INFINITY) { // No opaque fragment here, must be skyBox
            if (solidSkyBox) {
                frame->framebuffer[i] = solidSkyBox->value; // No need to compute UV
            } else {
                int x = i % frame->size.x, y= i / frame->size.x;
                skyBoxPixel(camera, frame, i, x, y);
            }
        } else { // Opaque fragment here
            if (frame->deferred && !f.face->material->flags.alphaCutout)
                f.baseColor = f.face->material->getBaseColor(f.uv, f.dUVdx, f.dUVdy);
            frame->framebuffer[i] = f.face->material->shade(f, frame->framebuffer[i], *scene);
        }

        // Transparent fragments
        for (uint32_t next = frame->transparencyHeads[i]; next != (uint32_t)-1;) {
            FragmentNode &node = frame->transparencyFragments[next];
            Fragment &f = node.f;

            fogTransparency(f, frame->framebuffer[i], z);

            frame->framebuffer[i] = f.face->material->shade(f, frame->framebuffer[i], *scene);

            z = f.z;
            next = node.next;
        }
        frame->zBuffer[i] = z; // z buffer isn't accurate after geometry pass because triangle order is reverse, so here we fix it
    }
}

void fogPass(uint n, uint i0, Camera *camera) {
    shared_ptr<Scene> scene = camera->obj->scene.lock();
    if(!scene) return;
    if(!scene->volume)
        return;

    RenderTarget *frame = camera->frame;
    for (size_t i = i0; i < frame->size.x * frame->size.y; i += n) {
        if(frame->zBuffer[i] == INFINITY && !(scene->volume && scene->volume->godRays)) // Sky-box pixels don't get fog unless its godRays
            continue;
        int x = i % frame->size.x, y= i / frame->size.x;
        float z = camera->frame->zBuffer[i];

        if(z == INFINITY) {
            if(scene->volume && scene->volume->godRays)
                z = camera->farClip;
            else
                return;
        }

        Vec3 cameraSpace = camera->screenSpaceToCameraSpace(x, y, z);

        camera->frame->framebuffer[i] = sampleFog(
            cameraSpace * camera->obj->transform, 
            camera->orthographic ? 
                camera->obj->globalPosition + Vec3{cameraSpace.x, cameraSpace.y, 0} * camera->obj->transformRotation :
                camera->obj->globalPosition,
            camera->frame->framebuffer[i],
            *scene,
            scene->volume
        );
    }
}
