#ifndef __MATERIAL_H__
#define __MATERIAL_H__
#include <string>
#include "color.h"
#include "miscTypes.h"

struct Volume {
    Color diffuse;
    Color emissive;
    Color transmission;
    Color intensity; // Not user facing

    void updateIntensity() {
        intensity = {
            (1.f - transmission.r) * transmission.a,
            (1.f - transmission.g) * transmission.a,
            (1.f - transmission.b) * transmission.a,
        };
    }
};

struct MaterialFlags {
    // Causes the polygons to be rasterized last, and allows polygons behind them to be partially visible. 
    // Slightly worse for performance, and polygons can't intersect.
    // Not needed for alpha cutout.
    bool transparent:1 = false;
    // Enable for flat materials. This allows back-faces which would normally be culled to be visible.
    bool doubleSided:1 = false;
    // If enabled, fragments with "base color" alpha < 0.5 will not be drawn. Useful for fences, leaves, etc.
    // If deferred rendering is enabled, this causes texture sampling to occur in geometry pass, reducing performance.
    bool alphaCutout:1 = false;
};

struct Scene;

class Material {
public:
    std::string name;
    MaterialFlags flags;
    bool needsTBN = false;
    Volume *volumeBack = nullptr, *volumeFront = nullptr;
    Material(std::string name, MaterialFlags flags, bool needsTBN, Volume *front = nullptr, Volume *back = nullptr) 
        : name(name), flags(flags), needsTBN(needsTBN), volumeBack(back), volumeFront(front) {}
    virtual Color shade(Fragment &f, Color previous, Scene *scene) = 0;
    virtual Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) = 0;
    virtual void GUI();
};

#endif /* __MATERIAL_H__ */
