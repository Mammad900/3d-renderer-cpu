#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "color.h"
#include "object.h"
#include "texture.h"
#include <memory>

class PhongMaterial : public Material {
public:
    shared_ptr<Texture<Color>> diffuse;
    shared_ptr<Texture<Color>> specular;
    shared_ptr<Texture<Color>> tint;
    shared_ptr<Texture<Color>> emissive;
    shared_ptr<Texture<Color>> environmentReflection;

    PhongMaterial(
        shared_ptr<Texture<Color>> diffuse, 
        shared_ptr<Texture<Color>> specular, 
        shared_ptr<Texture<Color>> tint, 
        shared_ptr<Texture<Color>> emissive, 
        shared_ptr<Texture<Color>> environmentReflection,
        shared_ptr<Texture<Vec3>>  normalMap,
        std::string name, MaterialFlags flags, 
        shared_ptr<Volume> front = nullptr, 
        shared_ptr<Volume> back = nullptr
    ) : 
        Material(name, flags, normalMap, front, back), 
        diffuse(diffuse), specular(specular), tint(tint), emissive(emissive), 
        environmentReflection(environmentReflection) { }
    
    PhongMaterial(
        std::string name, MaterialFlags flags, 
        shared_ptr<Volume> front = nullptr, 
        shared_ptr<Volume> back = nullptr
    ) : 
        Material(name, flags, nullptr, front, back), 
        diffuse(std::make_shared<SolidTexture<Color>>(Color{})), 
        specular(std::make_shared<SolidTexture<Color>>(Color{})), 
        tint(std::make_shared<SolidTexture<Color>>(Color{})), 
        emissive(std::make_shared<SolidTexture<Color>>(Color{})), 
        environmentReflection(std::make_shared<SolidTexture<Color>>(Color{})) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI();

    Color shade(Fragment &f, Color previous, Scene &scene);
};

#endif /* __PHONGMATERIAL_H__ */
