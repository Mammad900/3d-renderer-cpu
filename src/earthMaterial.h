#ifndef __EARTHMATERIAL_H__
#define __EARTHMATERIAL_H__
#include "phongMaterial.h"

SolidTexture<Color> *blankTexture() {
    return new SolidTexture<Color>({0, 0, 0, 0});
}

class EarthMaterial : public Material {
public:
    PhongMaterial *terrainMat;
    PhongMaterial *oceanMat;
    PhongMaterial *cloudMat;
    Texture<float> *oceanMask = new SolidTexture<float>(0);
    Texture<float> *cloudTexture = new SolidTexture<float>(0);

    EarthMaterial(std::string name) : Material(name, MaterialFlags::None, true) {
        PhongMaterialProps terrainProps{};
        terrainMat = new PhongMaterial(terrainProps, name+" Terrain", MaterialFlags::None);

        PhongMaterialProps oceanProps{};
        oceanMat = new PhongMaterial(oceanProps, name+" Terrain", MaterialFlags::None);

        PhongMaterialProps cloudProps{};
        cloudMat = new PhongMaterial(cloudProps, name+" Terrain", MaterialFlags::None);
    }

    Color shade(Fragment &f, Color previous) {
        bool isOcean = oceanMask->sample(f) > 0.5f;
        Color groundLighting = isOcean ? oceanMat->shade(f, previous) : terrainMat->shade(f, previous);
        f.baseColor = ((ImageTexture<Color>*)(cloudMat->mat.diffuse))->value; // Because it contains terrain diffuse and we want white
        Color cloudLighting = cloudMat->shade(f, previous);
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
