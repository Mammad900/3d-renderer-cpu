#include "camera.h"
#include "triangle.h"
#include "fog.h"
#include <imgui.h>
#include <functional>
#include <SFML/System/Clock.hpp>
#include <thread>
#include <condition_variable>

struct TransparentTriangle{
    float z;
    Triangle tri;
};

void deferredPass(uint n, uint i0, RenderTarget *frame);
void threadLoop(uint n, uint i0, RenderTarget *frame);
const uint numThreads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(numThreads);
std::vector<bool> jobReady(numThreads, false);
std::vector<std::condition_variable> cvs(numThreads);
std::mutex mtx;
bool shutdown = false;
bool init = false;

void Camera::render(RenderTarget *frame) {
    timing.clock.restart();
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
                Projection projectedVertices[mesh->vertices.size()];

                for (size_t j = 0; j < mesh->vertices.size(); j++) {
                    Vertex vV = mesh->vertices[j];

                    projectedVertices[j] = perspectiveProject(vV.position * obj->transform);
                    auto normal = (vV.normal * obj->transformRotation).normalized();
                    projectedVertices[j].normal = normal;
                }

                for (size_t j = 0; j < mesh->faces.size(); j++) {
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

    timing.renderPrepareTime.push(timing.clock);

#pragma endregion


#pragma region // ===== DRAW TRIANGLES =====

    for (auto &&tri : triangles)
        drawTriangle(frame, tri, frame->deferred);

    timing.geometryTime.push(timing.clock);


    // Deferred pass
    if(frame->deferred) {
        if(!init) {
            for (uint i = 0; i < numThreads; i++)
                threads[i] = std::thread(threadLoop, numThreads, i, frame);
            init = true;
        }
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (uint i = 0; i < numThreads; i++)
                jobReady[i] = true;
        }

        for (uint i = 0; i < numThreads; i++)
            cvs[i].notify_one();

        bool allDone = false;
        while(!allDone) {
            std::this_thread::yield();
            std::lock_guard<std::mutex> lock(mtx);
            allDone = std::all_of(jobReady.begin(), jobReady.end(), [](bool v) {return !v;});
        }
    }

    timing.lightingTime.push(timing.clock);

    auto &&compareZ = [](TransparentTriangle &a, TransparentTriangle &b){ return a.z > b.z; };
    std::sort(transparents.begin(), transparents.end(), compareZ);
    for (auto &&tri : transparents)
        drawTriangle(frame, tri.tri, false);
    
    timing.forwardTime.push(timing.clock);
    timing.clock.stop();

#pragma endregion

    if(scene->fogColor.a > 0)
        fog();
}

void threadLoop(uint n, uint i, RenderTarget *frame) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cvs[i].wait(lock, [&] { return jobReady[i] || shutdown; });
        if(shutdown) break;
        lock.unlock();
        deferredPass(n, i, frame);
        lock.lock();
        jobReady[i] = false;
    }
}

void deferredPass(uint n, uint i0, RenderTarget *frame) {
    for (size_t i = i0; i < frame->size.x * frame->size.y; i+=n) {
        if (frame->zBuffer[i] == INFINITY) // No fragment here
            continue;
        Fragment &f = frame->gBuffer[i];
        if (frame->deferred && !(f.mat->flags & MaterialFlags::AlphaCutout))
            f.baseColor = f.mat->getBaseColor(f.uv, f.dUVdx, f.dUVdy);
        frame->framebuffer[i] = f.mat->shade(f, frame->framebuffer[i]);
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
