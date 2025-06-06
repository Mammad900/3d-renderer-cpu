#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "color.h"
#include "texture.h"
#include <SFML/Graphics.hpp>
#include <string>

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

struct PhongMaterialProps {
    // Diffuse, aka albedo, aka base. If alpha < 0.5, it will not be drawn. (alpha cutout)
    Texture<Color> *diffuse = new SolidTexture<Color>({0,0,0,0});

    // Specular highlights. Alpha = 0 disables specular. Otherwise alpha = 10 * log2(shininess).
    Texture<Color> *specular = new SolidTexture<Color>({0,0,0,0});

    // Transparent materials: Filters light coming from behind the material, which is the existing pixels. 
    // Alpha is ignored. Useful for tinted glass, for example.
    //
    // Non-transparent materials: Controls subsurface scattering. 
    // In other words, light hitting the back of a flat object creates diffuse lighting visible at the front. 
    // Alpha controls how much the intensity depends on view direction. Most useful for leaves.
    Texture<Color> *tint = new SolidTexture<Color>({0,0,0,0});

    // This is added to the lighting calculation regardless of incoming light, as if the material emits this light itself.
    Texture<Color> *emissive = new SolidTexture<Color>({0,0,0,0});

    // Normal map, used to add detail that would otherwise require a lot of polygons
    optional<Texture<Vector3f>*> normalMap;

    // Displacement map, used to do Parallax [Occlusion] Mapping. Only used if a normal map is also defined.
    optional<Texture<float>*> displacementMap;

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
