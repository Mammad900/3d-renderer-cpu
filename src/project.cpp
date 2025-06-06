#include "project.h"
#include <SFML/System/Vector3.hpp>
#include <math.h>
#include "data.h"
#include "matrix.h"
using sf::Vector3f;
using std::sin, std::cos;
void makePerspectiveProjectionMatrix() {
    scene->projectionMatrix = makeRotationMatrix(-scene->camRotation);
    float S = 1 / tan(scene->fov * M_PI / 360);
    float f = -scene->farClip / (scene->farClip - scene->nearClip);
    matMul(scene->projectionMatrix.data(), (float[]){
        S, 0, 0, 0,
        0, S, 0, 0,
        0, 0, f,-1,
        0, 0,-f*scene->nearClip,0
    }, scene->projectionMatrix.data(), 4,4,4);
}

Projection perspectiveProject(Vector3f a) {
    a -= scene->cam;
    float vM[4] = {a.x, a.y, a.z, 1};
    matMul(vM, scene->projectionMatrix.data(), vM, 1, 4, 4);
    return Projection{
        .worldPos = a,
        .screenPos = Vector3f{vM[0] / vM[3], vM[1] / vM[3], -vM[3]},
    };
}