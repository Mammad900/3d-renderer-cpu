#ifndef __EARTHMATERIAL_H__
#define __EARTHMATERIAL_H__
#include "color.h"
#include "phongMaterial.h"
#include "texture.h"
#include "textureFiltering.h"
#include <memory>

inline SolidTexture<Color> *blankTexture() {
    return new SolidTexture<Color>({0, 0, 0, 0});
}

class EarthMaterial : public Material {
public:
    shared_ptr<PhongMaterial> terrainMat;
    shared_ptr<PhongMaterial> oceanMat;
    shared_ptr<PhongMaterial> cloudMat;
    shared_ptr<Texture<float>> oceanMask = std::make_shared<SolidTexture<float>>(0);
    shared_ptr<Texture<float>> cloudTexture = std::make_shared<SolidTexture<float>>(0);

    EarthMaterial(std::string name) : Material(name, MaterialFlags{}, true) {
        terrainMat = std::make_shared<PhongMaterial>(name+" Terrain", MaterialFlags{});

        oceanMat = std::make_shared<PhongMaterial>(name+" Terrain", MaterialFlags{});

        cloudMat = std::make_shared<PhongMaterial>(name+" Terrain", MaterialFlags{});
    }

    Color shade(Fragment &f, Color previous, Scene &scene) {
        bool isOcean = oceanMask->sample(f) > 0.5f;
        Color groundLighting = isOcean ? oceanMat->shade(f, previous, scene) : terrainMat->shade(f, previous, scene);
        f.baseColor = std::dynamic_pointer_cast<SolidTexture<Color>>(cloudMat->diffuse)->value; // Because it contains terrain diffuse and we want white
        Color cloudLighting = cloudMat->shade(f, previous, scene);
        float cloudIntensity = cloudTexture->sample(f);
        return groundLighting * (1 - cloudIntensity) +
               cloudLighting * cloudIntensity;
    }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return terrainMat->diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI() {
        terrainMat->diffuse->Gui("Terrain base");
        terrainMat->emissive->Gui("City lights");
        oceanMat->diffuse->Gui("Ocean diffuse");
        oceanMat->specular->Gui("Ocean specular");
        cloudMat->diffuse->Gui("Cloud diffuse");
        oceanMask->Gui("Ocean mask");
        cloudTexture->Gui("Cloud mask");

        ImGui::PushID(0);
        if(ImGui::TreeNode("Terrain material")) {
            terrainMat->GUI();
            ImGui::TreePop();
        }
        ImGui::PopID();

        ImGui::PushID(1);
        if(ImGui::TreeNode("Ocean material")) {
            oceanMat->GUI();
            ImGui::TreePop();
        }
        ImGui::PopID();

        ImGui::PushID(2);
        if(ImGui::TreeNode("Cloud material")) {
            cloudMat->GUI();
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
};

#endif /* __EARTHMATERIAL_H__ */
