#ifndef __MATERIAL_H__
#define __MATERIAL_H__
#include <string>
#include "color.h"
#include "miscTypes.h"

enum class MaterialFlags : uint8_t {
    None = 0,
    // Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
    // Slightly worse for performance, and polygons can't intersect.
    // Not needed for alpha cutout.
    Transparent = 1 << 0,
    // Enable for flat materials. This allows back-faces which would normally be culled to be visible.
    DoubleSided = 1 << 1,
    // If enabled, fragments with "base color" alpha < 0.5 will not be drawn. Useful for fences, leaves, etc.
    // If deferred rendering is enabled, this causes texture sampling to occur in geometry pass, reducing performance.
    AlphaCutout = 1 << 2,
};

constexpr bool operator& (const MaterialFlags &a, const MaterialFlags &b) {
    return static_cast<uint8_t>(a) & static_cast<uint8_t>(b);
}

constexpr MaterialFlags operator| (const MaterialFlags &a, const MaterialFlags &b) {
    return static_cast<MaterialFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr MaterialFlags operator^= (MaterialFlags &a, const MaterialFlags &b) {
    return a = static_cast<MaterialFlags>(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
}

class Material {
public:
    std::string name;
    MaterialFlags flags;
    bool needsTBN = false;
    Material(std::string name, MaterialFlags flags, bool needsTBN) : name(name), flags(flags), needsTBN(needsTBN) {}
    virtual Color shade(Fragment &f, Color previous) = 0;
    virtual Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) = 0;
    virtual void GUI();
};

#endif /* __MATERIAL_H__ */
