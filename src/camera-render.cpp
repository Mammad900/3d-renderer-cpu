#include "camera.h"
#include "triangle.h"
#include "fog.h"
#include <imgui.h>
#include <functional>
#include <SFML/System/Clock.hpp>
#include <thread>
#include <condition_variable>
#include <typeindex>

void deferredPass(uint n, uint i0, Camera *camera);
void threadLoop(uint n, uint i0);
const uint numThreads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(numThreads);
std::vector<Camera*> jobReady(numThreads, nullptr);
std::vector<std::condition_variable> cvs(numThreads);
std::mutex mtx;
bool shutdown = false;
bool init = false;

void Camera::render() {
    if(shadowMap) {
        drawSkyBox();

        makePerspectiveProjectionMatrix();

        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;

        buildTriangles(transparents, triangles);

        for (auto &&tri : triangles)
            drawTriangle(this, tri, tFrame->deferred);
    } 
    else {
        timing.clock.restart();
        drawSkyBox();
        timing.skyBoxTime.push(timing.clock);


        makePerspectiveProjectionMatrix();

        std::vector<Triangle> triangles;
        std::vector<TransparentTriangle> transparents;
        buildTriangles(transparents, triangles);

        timing.renderPrepareTime.push(timing.clock);


        for (auto &&tri : triangles)
            drawTriangle(this, tri, tFrame->deferred);

        timing.geometryTime.push(timing.clock);


        // Deferred pass
        if(tFrame->deferred) {
            if(!init) {
                for (uint i = 0; i < numThreads; i++)
                    threads[i] = std::thread(threadLoop, numThreads, i);
                init = true;
            }
            
            {
                std::lock_guard<std::mutex> lock(mtx);
                for (uint i = 0; i < numThreads; i++)
                    jobReady[i] = this;
            }

            for (uint i = 0; i < numThreads; i++)
                cvs[i].notify_one();

            bool allDone = false;
            while(!allDone) {
                std::this_thread::yield();
                std::lock_guard<std::mutex> lock(mtx);
                allDone = std::all_of(jobReady.begin(), jobReady.end(), [](Camera *v) {return !v;});
            }
        }

        timing.lightingTime.push(timing.clock);


        auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
        std::sort(transparents.begin(), transparents.end(), compareZ);
        for (auto &&tri : transparents)
            drawTriangle(this, tri.tri, false);
        
        timing.forwardTime.push(timing.clock);
        timing.clock.stop();


        if(obj->scene->fogColor.a > 0 && !tFrame->deferred)
            for (int y = 0; y < (int)tFrame->size.y; y++)
                for (int x = 0; x < (int)tFrame->size.x; x++)
                    fogPixel(x, y);
    }
}

void Camera::buildTriangles(
    std::vector<TransparentTriangle> &transparents,
    std::vector<Triangle> &triangles
) {
    std::function<void(Object *)> handleObject = [&](Object *obj) {
        for (auto &&comp : obj->components) {
            if (MeshComponent *meshComp = dynamic_cast<MeshComponent *>(comp)) {
                Mesh *mesh = meshComp->mesh;
                Projection projectedVertices[mesh->vertices.size()];

                for (size_t j = 0; j < mesh->vertices.size(); j++) {
                    Vertex vV = mesh->vertices[j];

                    projectedVertices[j] = perspectiveProject(vV.position * obj->transform);
                    auto normal = (vV.normal * obj->transformRotation).normalized();
                    projectedVertices[j].normal = normal;
                }

                for (size_t j = 0; j < mesh->faces.size(); j++) {
                    Face &face = mesh->faces[j];
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

    for (auto &&obj : obj->scene->objects)
        handleObject(obj);
}

void Camera::drawSkyBox() {
    SolidTexture<Color> *solidSkyBox = nullptr;
    { // dynamic_cast alone doesn't work because we don't want derived classes
        std::type_index ti(typeid(*obj->scene->skyBox));
        if (ti == std::type_index(typeid(SolidTexture<Color>)))
            solidSkyBox = dynamic_cast<SolidTexture<Color> *>(obj->scene->skyBox);
    }
    for (uint y = 0; y < tFrame->size.y; y++) {
        for (uint x = 0; x < tFrame->size.x; x++) {
            size_t i = y * tFrame->size.x + x;
            if(!shadowMap) {
                if (solidSkyBox) {
                    tFrame->framebuffer[i] = solidSkyBox->value; // No need to compute UV
                } else {
                    Vector2f worldPos {x / (float)tFrame->size.x, y / (float)tFrame->size.y};
                    worldPos = (Vector2f{0.5, 0.5} - worldPos) * 2.0f * tanHalfFov;
                    Vector3f lookVector = Vector3f{worldPos.x, worldPos.y, 1} * obj->transformRotation;
                    lookVector = lookVector.normalized();

                    Vector2f uv {
                        0.5f +(atan2f(lookVector.z, lookVector.x) / (2.0f * M_PIf)),
                        0.5f - (asinf(lookVector.y) / M_PIf)
                    };

                    tFrame->framebuffer[i] = obj->scene->skyBox->sample(uv, {0, 0}, {0, 0});
                }
            }
            tFrame->zBuffer[i] = INFINITY;
        }
    }
}

void threadLoop(uint n, uint i) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cvs[i].wait(lock, [&] { return jobReady[i] || shutdown; });
        if(shutdown) break;
        lock.unlock();
        deferredPass(n, i, jobReady[i]);
        lock.lock();
        jobReady[i] = nullptr;
    }
}

void deferredPass(uint n, uint i0, Camera *camera) {
    RenderTarget *frame = camera->tFrame;
    for (size_t i = i0; i < frame->size.x * frame->size.y; i += n) {
        if (frame->zBuffer[i] == INFINITY) { // No fragment here
            if(camera->obj->scene->godRays)
                camera->fogPixel(i % frame->size.x, i / frame->size.x);
            continue;
        }
        Fragment &f = frame->gBuffer[i];
        if (frame->deferred && !f.face->material->flags.alphaCutout)
            f.baseColor = f.face->material->getBaseColor(f.uv, f.dUVdx, f.dUVdy);
        frame->framebuffer[i] = f.face->material->shade(f, frame->framebuffer[i], camera->obj->scene);
        if(camera->obj->scene->fogColor.a > 0)
            frame->framebuffer[i] = sampleFog(f.worldPos, camera->obj->globalPosition, frame->framebuffer[i], camera->obj->scene);
    }
}

void shutdownThreads() {
    if(!init)
        return;
    {
        std::lock_guard<std::mutex> lock(mtx);
        shutdown = true;
    }
    for(auto &cv : cvs)
        cv.notify_all();
    for(auto &t : threads)
        t.join();
}
