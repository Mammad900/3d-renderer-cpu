#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "color.h"
#include <SFML/Graphics.hpp>
#include <string>

using sf::Vector3f, sf::Vector2f;

enum MaterialFlags : uint8_t {
    // Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
    // Slightly worse for performance, and polygons can't intersect.
    // Not needed for alpha cutout.
    Transparent = 1,
    // Enable for transparent materials. This allows back-faces which would normally be culled to be visible.
    DisableBackfaceCulling = 2
};

struct Material {
    // Diffuse color. If alpha = 0, diffuse is disabled, even the texture. Otherwise, alpha is unused.
    Color diffuseColor;
    // Texture that replaces diffuse color. If not null, diffuse color (except alpha) will be ignored. 
    // This texture's alpha channel is handled differently: if a pixel's alpha < 0.5, it will not be drawn. (alpha cutout)
    sf::Image *diffuseTexture;
    // Specular highlights color. Alpha = 0 disables specular. Otherwise alpha = 10 * log2(shininess).
    Color specularColor;
    // Texture for specular. Behaves the same way and overrides the color field.
    sf::Image *specularTexture;
    // Transparent materials only: filters light coming from behind the material. Useful for tinted glass, for example.
    Color tintColor;
    sf::Image *tintTexture;
    // Emissive: this color is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.
    Color emissiveColor;
    sf::Image *emissiveTexture;
    MaterialFlags flags;
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
    Vector3f rotation;
    Vector3f direction;
    // Use alpha to set intensity
    Color color;
    // If false, direction is the vector of the light. If true, direction is its 3D world position.
    bool isPointLight;
};

#endif /* __OBJECT_H__ */
