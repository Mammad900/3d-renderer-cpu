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
        PhongMaterialProps terrainProps{};
        terrainMat = std::make_shared<PhongMaterial>(terrainProps, name+" Terrain", MaterialFlags{});

        PhongMaterialProps oceanProps{};
        oceanMat = std::make_shared<PhongMaterial>(oceanProps, name+" Terrain", MaterialFlags{});

        PhongMaterialProps cloudProps{};
        cloudMat = std::make_shared<PhongMaterial>(cloudProps, name+" Terrain", MaterialFlags{});
    }

    Color shade(Fragment &f, Color previous, Scene &scene) {
        bool isOcean = oceanMask->sample(f) > 0.5f;
        Color groundLighting = isOcean ? oceanMat->shade(f, previous, scene) : terrainMat->shade(f, previous, scene);
        f.baseColor = std::dynamic_pointer_cast<SolidTexture<Color>>(cloudMat->mat.diffuse)->value; // Because it contains terrain diffuse and we want white
        Color cloudLighting = cloudMat->shade(f, previous, scene);
        float cloudIntensity = cloudTexture->sample(f);
        return groundLighting * (1 - cloudIntensity) +
               cloudLighting * cloudIntensity;
    }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return terrainMat->mat.diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI() {
        terrainMat->mat.diffuse->Gui("Terrain base");
        terrainMat->mat.emissive->Gui("City lights");
        oceanMat->mat.diffuse->Gui("Ocean diffuse");
        oceanMat->mat.specular->Gui("Ocean specular");
        cloudMat->mat.diffuse->Gui("Cloud diffuse");
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
