#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "color.h"
#include <SFML/Graphics.hpp>
#include <string>

using sf::Vector3f, sf::Vector2f, sf::Vector2u, std::optional;


enum MaterialFlags : uint8_t {
    None = 0,
    // Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
    // Slightly worse for performance, and polygons can't intersect.
    // Not needed for alpha cutout.
    Transparent = 1 << 0,
    // Enable for flat materials. This allows back-faces which would normally be culled to be visible.
    DoubleSided = 1 << 1
};

class Material;

struct Fragment {
    Vector3f position;
    Vector3f normal;
    Vector3f tangent;
    Vector3f bitangent;
    Vector2f uv;
    Vector2f uv_p;
    Color baseColor;
    Material *mat;
    bool isBackFace;
};

class Material {
public:
    std::string name;
    MaterialFlags flags;
    bool needsTBN = false;
    Material(std::string name, MaterialFlags flags, bool needsTBN) : name(name), flags(flags), needsTBN(needsTBN) {}
    virtual Color shade(Fragment &f, Color previous) = 0;
    virtual Color getBaseColor(Vector2f uv, Vector2f uv_p) = 0;
    virtual void GUI() = 0;
};

template <typename T>
struct Texture {
    // It's actually an atlas size*2-1 big, that also contains mipmaps.
    T* pixels;
    Vector2u size;
    Vector2u atlasSize;
};
struct MaterialMap {
    Color color;
    optional<Texture<Color>> texture;
};

struct PhongMaterialProps {
    // Diffuse, aka albedo, aka base. If alpha < 0.5, it will not be drawn. (alpha cutout)
    MaterialMap diffuse;

    // Specular highlights. Alpha = 0 disables specular. Otherwise alpha = 10 * log2(shininess).
    MaterialMap specular;

    // Transparent materials: Filters light coming from behind the material, which is the existing pixels. 
    // Alpha is ignored. Useful for tinted glass, for example.
    //
    // Non-transparent materials: Controls subsurface scattering. 
    // In other words, light hitting the back of a flat object creates diffuse lighting visible at the front. 
    // Alpha controls how much the intensity depends on view direction. Most useful for leaves.
    MaterialMap tint;

    // This is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.
    MaterialMap emissive;

    // Normal map, used to add detail that would otherwise require a lot of polygons
    optional<Texture<Vector3f>> normalMap;

    // Displacement map, used to do Parallax [Occlusion] Mapping. Only used if a normal map is also defined.
    optional<Texture<float>> displacementMap;

    // Parallax Occlusion mapping steps, the higher the slower. 0 means simple Parallax Mapping. Only used if displacement map is defined.
    uint8_t POM;
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
