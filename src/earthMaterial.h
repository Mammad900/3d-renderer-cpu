#ifndef __EARTHMATERIAL_H__
#define __EARTHMATERIAL_H__
#include "baseMaterial.h"

class EarthMaterial : public Material {
public:
    BaseMaterial *terrain;
    BaseMaterial *ocean;
    BaseMaterial *cloud;
    Texture<float> oceanMask;
    Texture<float> cloudTexture;

    EarthMaterial() {
        sf::Image diffuse("assets/earth-diffuse-8k.jpg");
        sf::Image oceanMask("assets/earth-specular-8k.jpg");
        sf::Image lights("assets/earth-lights-8k.jpg");
        sf::Image clouds("assets/earth-clouds-4k.png");
        sf::Image normalMap("assets/earth-normalmap-8k.jpg");

        this->oceanMask = loadFloatTexture(oceanMask);
        this->cloudTexture = loadFloatTexture(clouds);

        BaseMaterialProps terrainProps{
            .diffuse = {
                .color = {1, 1, 1, 1},
                .texture = loadColorTexture(diffuse),
            },
            .emissive = { 
                .color = {1, 1, 1, 1},
                .texture = loadColorTexture(lights),
            },
            .normalMap = loadVectorTexture(normalMap),
        };
        terrain = new BaseMaterial(terrainProps, MaterialFlags::None);

        BaseMaterialProps oceanProps{
            .diffuse = {.color = {1,1,1,1}},
            .specular = {.color = {0.275, 0.38, 0.6, 0.16}}
        };
        ocean = new BaseMaterial(oceanProps, MaterialFlags::None);

        BaseMaterialProps cloudProps{.diffuse = {.color = {1, 1, 1, 1}}};
        cloud = new BaseMaterial(cloudProps, MaterialFlags::None);

        this->needsTBN = true;
    }

    Color shade(Fragment &f, Color previous) {
        bool isOcean = textureFilter(oceanMask, f.uv) > 0.5f;
        Color groundLighting = isOcean ? ocean->shade(f, previous) : terrain->shade(f, previous);
        f.baseColor = cloud->mat.diffuse.color; // Because it contains terrain diffuse and we want white
        Color cloudLighting = cloud->shade(f, previous);
        float cloudIntensity = textureFilter(cloudTexture, f.uv);
        return groundLighting * (1 - cloudIntensity) +
               cloudLighting * cloudIntensity;
    }

    Color getBaseColor(Vector2f uv, Vector2f uv_p) {
        return textureFilter(terrain->mat.diffuse.texture.value(), uv);
    }

    void GUI() {
        ImGui::ColorEdit4("Terrain base", (float*)&terrain->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("City lights", (float*)&terrain->mat.emissive.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Ocean diffuse", (float*)&ocean->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Ocean specular", (float*)&ocean->mat.specular.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Cloud diffuse", (float*)&cloud->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    }
};

#endif /* __EARTHMATERIAL_H__ */
