#include "camera.h"
#include "data.h"
#include "triangle.h"
#include "vector3.h"
#include <SFML/System/Vector2.hpp>
#include <imgui.h>
#include <SFML/System/Clock.hpp>

void Camera::update() {

}

Projection Camera::project(Vec3 a) {
    float vM[4] = {a.x, a.y, a.z, 1};
    matMul(vM, projectionMatrix.data(), vM, 1, 4, 4);
    return Projection{
        .worldPos = a,
        .screenPos = orthographic ? -Vec3{vM[0], vM[1], vM[3]} : Vec3{vM[0] / vM[3], vM[1] / vM[3], -vM[3]},
    };
}

sf::Image Camera::getRenderedFrame(RenderMode renderMode) {
    sf::Image img(frame->size);
    for (unsigned int y = 0; y < frame->size.y; y++)
        for (unsigned int x = 0; x < frame->size.x; x++) {
            size_t i = y * frame->size.x + x;
            sf::Color c{0,0,0};

            if (renderMode == RenderMode::normal) {
                Color pixel = frame->framebuffer[i];
                c = pixel.reinhardtTonemap(whitePoint==0 ? maximumColor : whitePoint);
            }
            else if (renderMode == RenderMode::zBuffer) {
                // Z buffer range is really display-to-end-user unfriendly
                float z = frame->zBuffer[i] * 20.0f;
                c = sf::Color(z, z, z);
            }
            else if (renderMode == RenderMode::gBufferPosition) {
                Vec3 pos = frame->gBuffer[i].worldPos * 20.0f;
                c = sf::Color(pos.x+127, pos.y+127, pos.z+127);
            }
            else if (renderMode == RenderMode::gBufferNormal) {
                Vec3 pos = frame->gBuffer[i].normal * 100.0f;
                c = sf::Color(pos.x+127, pos.y+127, pos.z+127);
            }
            else if (renderMode == RenderMode::gBufferTangent) {
                Vec3 pos = frame->gBuffer[i].tangent * 100.0f;
                c = sf::Color(pos.x+127, pos.y+127, pos.z+127);
            }
            else if (renderMode == RenderMode::gBufferBitangent) {
                Vec3 pos = frame->gBuffer[i].bitangent * 100.0f;
                c = sf::Color(pos.x+127, pos.y+127, pos.z+127);
            }
            else if (renderMode == RenderMode::gBufferViewDir) {
                Vec3 pos = frame->gBuffer[i].viewDir * 100.0f;
                c = sf::Color(pos.x+127, pos.y+127, pos.z+127);
            }

            img.setPixel({x, y}, c);
        }
    return img;
}

Vec3 Camera::screenSpaceToCameraSpace(int x, int y) { 
    size_t i = x + frame->size.x * y;
    float z = frame->zBuffer[i];
    return screenSpaceToCameraSpace(x, y, z);
}

Vec3 Camera::screenSpaceToCameraSpace(int x, int y, float z) { 
    Vector2f worldPos{x / (float)frame->size.x, y / (float)frame->size.y};
    worldPos = (Vector2f{0.5, 0.5} - worldPos).componentWiseMul({tanHalfFov, tanHalfFovVertical}) * (2.0f * (orthographic ? 1 : z));
    return Vec3{worldPos.x, worldPos.y, z};
}

Vec3 Camera::screenSpaceToWorldSpace(int x, int y) {
    return screenSpaceToCameraSpace(x, y) * obj->transform;
}

Vec3 Camera::screenSpaceToWorldSpace(int x, int y, float z) {
    return screenSpaceToCameraSpace(x, y, z) * obj->transform;
}

void Camera::makeProjectionMatrix() {
    tanHalfFov = orthographic ? fov : tan(fov * M_PI / 360);
    tanHalfFovVertical = frame ? tanHalfFov * frame->size.y / frame->size.x : tanHalfFov;
    float SH = 1 / tanHalfFov;
    float SV = 1 / tanHalfFovVertical;
    float f = -farClip / (farClip - nearClip);
    TransformMatrix translate {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -obj->globalPosition.x, -obj->globalPosition.y, -obj->globalPosition.z, 1,
    }, project {
        SH, 0, 0, 0,
        0, SV, 0, 0,
        0, 0, f,-1,
        0, 0,-f*nearClip,0
    };
    projectionMatrix = translate * transposeMatrix(obj->transformRotation) * project;
}

void Camera::GUI() {
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene || currentWindow->scene != scene) return;
    
    if(ImGui::Button("Set as scene camera")) {
        currentWindow->camera->frame = nullptr;
        currentWindow->camera = shared_from_this();
        frame = currentWindow->frame.get();
    }
}
