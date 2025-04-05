#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "color.h"
#include <SFML/System.hpp>
#include <string>

using sf::Vector3f, sf::Vector2f;

struct Material {
    Color diffuse;
    Color specular;
    float shinyness;
};

struct Vertex {
    Vector3f position;
    Vector2f uv;
    Vector3f normal;
};

struct Face {
    uint16_t v1, v2, v3;
    Material *material;
    bool invert;
};

struct Mesh {
    std::string label;
    Vertex *vertices;
    Face *faces;
    uint16_t n_vertices, n_faces;
};

struct Object {
    Mesh *mesh;
    Vector3f position;
    Vector3f rotation;
    Vector3f scale;
};

struct Light {
    Vector3f direction;
    Vector3f normal;
    Color color;
};

#endif /* __OBJECT_H__ */
