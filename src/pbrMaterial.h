#ifndef __PBRMATERIAL_H__
#define __PBRMATERIAL_H__

#include "material.h"

class PBRMaterial : public Material {
  public:
    Texture<Color> *albedo;
    Texture<float> *metallic;
    Texture<float> *roughness;
    Texture<float> *ambientOcclusion;

    PBRMaterial(
        std::string name, MaterialFlags flags, Texture<Color> *albedo,
        Texture<float> *metallic, Texture<float> *roughness,
        Texture<float> *ambientOcclusion
    )
        : Material(name, flags, false), albedo(albedo), metallic(metallic),
          roughness(roughness), ambientOcclusion(ambientOcclusion) {}

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return albedo->sample(uv, dUVdx, dUVdy);
    }

    void GUI();
    Color shade(Fragment &f, Color previous, Scene *scene);
};

#endif /* __PBRMATERIAL_H__ */
