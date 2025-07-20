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
