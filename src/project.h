#ifndef __PROJECT_H__
#define __PROJECT_H__
#include <SFML/System/Vector3.hpp>
#include <math.h>
#include "data.h"
#include "matrix.h"
using sf::Vector3f;
using std::sin, std::cos;

struct Projection {
    Vector3f worldPos;
    Vector3f screenPos;
    float w;
    Vector3f normal;
};

float projectionMatrix[16];

void makePerspectiveProjectionMatrix() {
    makeRotationMatrix(-camRotation, projectionMatrix);
    float S = 1 / tan(fov * M_PI / 360);
    float f = -farClip / (farClip - nearClip);
    matMul(projectionMatrix, (float[]){
        S, 0, 0, 0,
        0, S, 0, 0,
        0, 0, f,-1,
        0, 0,-f*nearClip,0
    }, projectionMatrix, 4,4,4);
}

Projection perspectiveProject(Vector3f a) {
    a -= cam;
    float vM[4] = {a.x, a.y, a.z, 1};
    matMul(vM, projectionMatrix, vM, 1, 4, 4);
    return Projection{
        .worldPos = a,
        .screenPos = Vector3f{vM[0], vM[1], vM[2]} / vM[3],
        .w = vM[3]
    };
}

#endif /* __PROJECT_H__ */
