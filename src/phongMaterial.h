#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "object.h"


class PhongMaterial : public Material {
    using enum MaterialFlags;

public:
    PhongMaterialProps mat;

    PhongMaterial(PhongMaterialProps &mat, std::string name, MaterialFlags flags) 
        : Material(name, flags, mat.normalMap.has_value()), mat(mat) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return mat.diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI();

    Color shade(Fragment &f, Color previous);
};

#endif /* __PHONGMATERIAL_H__ */
