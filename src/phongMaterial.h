#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "object.h"

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

class PhongMaterial : public Material {
public:
    PhongMaterialProps mat;

    PhongMaterial(const PhongMaterialProps &mat, std::string name, MaterialFlags flags) 
        : Material(name, flags, mat.normalMap.has_value()), mat(mat) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return mat.diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI();

    Color shade(Fragment &f, Color previous, Scene *scene);
};

#endif /* __PHONGMATERIAL_H__ */
