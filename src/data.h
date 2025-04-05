#ifndef __DATA_H__
#define __DATA_H__

#include "object.h"
#include <SFML/Graphics.hpp>

using sf::Vector3f, sf::Vector2u;

// clang-format off

static Material cubeMaterial = {
    Color{1.0f, 1.0f, 1.0f, 1.0f}, // Diffuse (white)
    Color{1.0f, 1.0f, 1.0f, 1.0f}, // Specular (White)
    32.0f                           // Shininess
};

static Mesh cubeMesh = {
    "Cube", // Label
    (Vertex[]){
        // Front face (Red)
        {{-1, -1,  1}, {0, 0}}, // v0
        {{ 1, -1,  1}, {1, 0}}, // v1
        {{ 1,  1,  1}, {1, 1}}, // v2
        {{-1,  1,  1}, {0, 1}}, // v3

        // Back face (Green)
        {{-1, -1, -1}, {0, 0}}, // v4
        {{ 1, -1, -1}, {1, 0}}, // v5
        {{ 1,  1, -1}, {1, 1}}, // v6
        {{-1,  1, -1}, {0, 1}}, // v7

        // Top face (Blue)
        {{-1,  1,  1}, {0, 0}}, // v8
        {{ 1,  1,  1}, {1, 0}}, // v9
        {{ 1,  1, -1}, {1, 1}}, // v10
        {{-1,  1, -1}, {0, 1}}, // v11

        // Bottom face (Yellow)
        {{-1, -1,  1}, {0, 0}}, // v12
        {{ 1, -1,  1}, {1, 0}}, // v13
        {{ 1, -1, -1}, {1, 1}}, // v14
        {{-1, -1, -1}, {0, 1}}, // v15

        // Left face (Purple)
        {{-1, -1, -1}, {0, 0}}, // v16
        {{-1, -1,  1}, {1, 0}}, // v17
        {{-1,  1,  1}, {1, 1}}, // v18
        {{-1,  1, -1}, {0, 1}}, // v19

        // Right face (Orange)
        {{ 1, -1, -1}, {0, 0}}, // v20
        {{ 1, -1,  1}, {1, 0}}, // v21
        {{ 1,  1,  1}, {1, 1}}, // v22
        {{ 1,  1, -1}, {0, 1}}, // v23
    },
    // Faces for the cube (two triangles per face, 6 faces)
    (Face[]){
        // Front face
        {2, 1, 0, &cubeMaterial}, // Triangle 1
        {3, 2, 0, &cubeMaterial}, // Triangle 2

        // Back face
        {4, 5, 6, &cubeMaterial}, // Triangle 3
        {4, 6, 7, &cubeMaterial}, // Triangle 4

        // Top face
        {10, 9, 8, &cubeMaterial}, // Triangle 5
        {11, 10, 8, &cubeMaterial}, // Triangle 6

        // Bottom face
        {12, 13, 14, &cubeMaterial}, // Triangle 7
        {12, 14, 15, &cubeMaterial}, // Triangle 8

        // Left face
        {18, 17, 16, &cubeMaterial}, // Triangle 9
        {19, 18, 16, &cubeMaterial}, // Triangle 10

        // Right face
        {20, 21, 22, &cubeMaterial}, // Triangle 11
        {20, 22, 23, &cubeMaterial}, // Triangle 12
    },
    24, // Number of vertices (6 faces * 4 vertices = 24)
    12 // Number of faces (6 faces * 2 triangles = 12)
};
// clang-format on

std::vector<Light> lights = {
    Light{
        .direction={0,-1,0},
        .color=Color{1,1,1,1}
    }
};
Color ambientLight = {1, 1, 1, 0.1};

std::vector<Material*> materials = {&cubeMaterial};
std::vector<Mesh*> meshes = {&cubeMesh};

std::vector<Object> objects;
//= {{&cubeMesh, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}}};

Vector3f cam = {0, 0, -5};       // Camera at (0, 0, -5)
Vector3f camRotation = {0, 0, 0}; // No rotation (identity rotation)
float nearClip = 0.1, farClip = 100;
float fov = 45;
constexpr Vector2u frameSize = {500, 500};

Color framebuffer[frameSize.x * frameSize.y];
float zBuffer[frameSize.x * frameSize.y];

int renderMode = 0;
bool backFaceCulling = true;
bool reverseAllFaces = false;

#endif /* __DATA_H__ */
