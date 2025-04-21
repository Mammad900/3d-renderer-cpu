#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector3f, sf::Vector2u;

// clang-format off

// static Material cubeMaterial = {
//     .diffuseColor= Color{1.0f, 1.0f, 1.0f, 1.0f},
//     .diffuseTexture= nullptr,
//     .specularColor= Color{1.0f, 1.0f, 1.0f, 50.0f},
// };

std::vector<Light> lights = {
    // Light{
    //     .direction={0,-1,0},
    //     .color=Color{1,1,1,1}
    // }
};
Color ambientLight = {1, 1, 1, 0.1};

std::vector<Material*> materials;
std::vector<Mesh*> meshes;

std::vector<Object> objects;
//= {{&cubeMesh, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}}};

Vector3f cam = {0, 0, 0};
Vector3f camRotation = {0, 0, 0};
Vector3f camDirection;
float nearClip = 0.1, farClip = 100;
float fov = 90;
constexpr Vector2u frameSize = {500, 500};

Color framebuffer[frameSize.x * frameSize.y];
float zBuffer[frameSize.x * frameSize.y];
float maximumColor;

int renderMode = 0;
bool backFaceCulling = true;
bool reverseAllFaces = false;
bool fullBright = false;
bool wireFrame = false;
bool gammaCorrection = false;
float whitePoint = 1;
Color fogColor = {0,0,0,0};

#endif /* __DATA_H__ */
