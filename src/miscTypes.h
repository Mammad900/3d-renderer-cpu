#ifndef __MISCTYPES_H__
#define __MISCTYPES_H__
#include <SFML/Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include "color.h"
#include "texture.h"

using sf::Vector3f, sf::Vector2f, sf::Vector2u, sf::Vector2i, std::optional;

enum class MaterialFlags : uint8_t {
    None = 0,
    // Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
    // Slightly worse for performance, and polygons can't intersect.
    // Not needed for alpha cutout.
    Transparent = 1 << 0,
    // Enable for flat materials. This allows back-faces which would normally be culled to be visible.
    DoubleSided = 1 << 1
};

constexpr bool operator& (MaterialFlags a, MaterialFlags b) {
    return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
}

constexpr MaterialFlags operator| (MaterialFlags a, MaterialFlags b) {
    return static_cast<MaterialFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

enum class TextureFilteringMode : uint8_t {
    None = 0,
    NearestNeighbor = 1,
    Bilinear = 2,
    Trilinear = 3,
};

class Material;

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
    Material *mat;
    bool isBackFace, inside;
};

class Material {
public:
    std::string name;
    MaterialFlags flags;
    bool needsTBN = false;
    Material(std::string name, MaterialFlags flags, bool needsTBN) : name(name), flags(flags), needsTBN(needsTBN) {}
    virtual Color shade(Fragment &f, Color previous) = 0;
    virtual Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) = 0;
    virtual void GUI() = 0;
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
    Vertex *vertices;
    Face *faces;
    uint16_t n_vertices, n_faces;
};

#endif /* __MISCTYPES_H__ */
