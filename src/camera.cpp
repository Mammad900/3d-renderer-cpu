#include "camera.h"
#include "triangle.h"
#include <imgui.h>
#include <functional>
#include <SFML/System/Clock.hpp>

Projection Camera::perspectiveProject(Vec3 a) {
    Vec3 b = a - obj->globalPosition;
    float vM[4] = {b.x, b.y, b.z, 1};
    matMul(vM, projectionMatrix.data(), vM, 1, 4, 4);
    return Projection{
        .worldPos = a,
        .screenPos = Vec3{vM[0] / vM[3], vM[1] / vM[3], -vM[3]},
    };
}

sf::Image Camera::getRenderedFrame(int renderMode) { 
    sf::Image img(frame->size);
    for (unsigned int y = 0; y < frame->size.y; y++)
        for (unsigned int x = 0; x < frame->size.x; x++)
            if (renderMode == 0) { // Frame buffer
                Color pixel = frame->framebuffer[y * frame->size.x + x];
                img.setPixel({x, y}, pixel.reinhardtTonemap(whitePoint==0 ? obj->scene->maximumColor : whitePoint));
            }
            else if (renderMode == 1) { // Z buffer
                // Z buffer range is really display-to-end-user unfriendly
                float z = frame->zBuffer[y * frame->size.x + x] * 20.0f;
                img.setPixel({x, y}, sf::Color(z, z, z));
            }
    return img;
}

Vec3 Camera::screenSpaceToCameraSpace(int x, int y) { 
    size_t i = x + tFrame->size.x * y;
    float z = tFrame->zBuffer[i];
    return screenSpaceToCameraSpace(x, y, z);
}

Vec3 Camera::screenSpaceToCameraSpace(int x, int y, float z) { 
    Vector2f worldPos{x / (float)tFrame->size.x, y / (float)tFrame->size.y};
    worldPos = (Vector2f{0.5, 0.5} - worldPos) * 2.0f * z * tanHalfFov;
    return Vec3{worldPos.x, worldPos.y, z};
}

Vec3 Camera::screenSpaceToWorldSpace(int x, int y) {
    return screenSpaceToCameraSpace(x, y) * obj->transform;
}

Vec3 Camera::screenSpaceToWorldSpace(int x, int y, float z) {
    return screenSpaceToCameraSpace(x, y, z) * obj->transform;
}

void Camera::makePerspectiveProjectionMatrix() {
    float S = 1 / (tanHalfFov = tan(fov * M_PI / 360));
    float f = -farClip / (farClip - nearClip);
    matMul(transposeMatrix(obj->transformRotation).data(), (float[]){
        S, 0, 0, 0,
        0, S, 0, 0,
        0, 0, f,-1,
        0, 0,-f*nearClip,0
    }, projectionMatrix.data(), 4,4,4);
}

void Camera::GUI() {
    if(ImGui::Button("Set as scene camera")) {
        obj->scene->camera->tFrame = nullptr;
        obj->scene->camera = this;
        tFrame = frame;
    }
}
