#ifndef __MISCTYPES_H__
#define __MISCTYPES_H__
#include <SFML/Graphics.hpp>
#include <string>
#include "color.h"
#include "texture.h"
#include "material.h"

using sf::Vector3f, sf::Vector2f, sf::Vector2u, sf::Vector2i, std::optional, std::shared_ptr, std::unique_ptr, std::vector;

enum class TextureFilteringMode : uint8_t {
    None = 0,
    NearestNeighbor = 1,
    Bilinear = 2,
    Trilinear = 3,
};

class Material;

struct Projection {
    Vector3f worldPos;
    Vector3f screenPos;
    Vector3f normal;
};

struct Vertex {
    Vector3f position;
    Vector2f uv;
    Vector3f normal;
};

struct Face {
    uint16_t v1, v2, v3;
    Material *material;
};

struct Mesh {
    std::string label;
    vector<Vertex> vertices;
    vector<Face> faces;
    bool flatShading = false;

    Mesh(const std::string& label = "", const vector<Vertex>& vertices = {}, const vector<Face>& faces = {}, bool flatShading = false)
        : label(label), vertices(vertices), faces(faces), flatShading(flatShading) {}
};

struct Fragment {
    Vector2i screenPos;
    float z;
    Vector3f worldPos;
    Vector3f normal;
    Vector3f tangent;
    Vector3f bitangent;
    Vector2f uv;
    Vector2f dUVdx;
    Vector2f dUVdy;
    Color baseColor;
    Face *face;
    bool isBackFace, inside;
};

struct Triangle {
    Projection s1, s2, s3;
    Vector2f uv1, uv2, uv3;
    Material *mat;
    Face *face;
    Mesh *mesh;
    bool cull;
};

struct TransparentTriangle{
    float z;
    Triangle tri;
};

#endif /* __MISCTYPES_H__ */
