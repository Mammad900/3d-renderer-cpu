#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "object.h"

struct PhongMaterialProps {
    shared_ptr<Texture<Color>> diffuse = std::make_shared<SolidTexture<Color>>(Color{1,1,1,1});

    shared_ptr<Texture<Color>> specular = std::make_shared<SolidTexture<Color>>(Color{0,0,0,0});

    shared_ptr<Texture<Color>> tint = std::make_shared<SolidTexture<Color>>(Color{0,0,0,0});

    shared_ptr<Texture<Color>> emissive = std::make_shared<SolidTexture<Color>>(Color{0,0,0,0});

    Color environmentReflection{0,0,0,0};

    shared_ptr<Texture<Vec3>>  normalMap;

    shared_ptr<Texture<float>> displacementMap;

    uint8_t POM;
};

class PhongMaterial : public Material {
public:
    PhongMaterialProps mat;

    PhongMaterial(const PhongMaterialProps &mat, std::string name, MaterialFlags flags, shared_ptr<Volume> front = nullptr, shared_ptr<Volume> back = nullptr) 
        : Material(name, flags, mat.normalMap != nullptr, front, back), mat(mat) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return mat.diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI();

    Color shade(Fragment &f, Color previous, Scene &scene);
};

#endif /* __PHONGMATERIAL_H__ */
