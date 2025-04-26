#ifndef __EARTHMATERIAL_H__
#define __EARTHMATERIAL_H__
#include "phongMaterial.h"

class EarthMaterial : public Material {
public:
    PhongMaterial *terrainMat;
    PhongMaterial *oceanMat;
    PhongMaterial *cloudMat;
    Texture<float> oceanMask;
    Texture<float> cloudTexture;

    EarthMaterial(std::string name) : Material(name, MaterialFlags::None, true) {
        PhongMaterialProps terrainProps{};
        terrainMat = new PhongMaterial(terrainProps, name+" Terrain", MaterialFlags::None);

        PhongMaterialProps oceanProps{};
        oceanMat = new PhongMaterial(oceanProps, name+" Terrain", MaterialFlags::None);

        PhongMaterialProps cloudProps{};
        cloudMat = new PhongMaterial(cloudProps, name+" Terrain", MaterialFlags::None);
    }

    Color shade(Fragment &f, Color previous) {
        bool isOcean = textureFilter(oceanMask, f.uv) > 0.5f;
        Color groundLighting = isOcean ? oceanMat->shade(f, previous) : terrainMat->shade(f, previous);
        f.baseColor = cloudMat->mat.diffuse.color; // Because it contains terrain diffuse and we want white
        Color cloudLighting = cloudMat->shade(f, previous);
        float cloudIntensity = textureFilter(cloudTexture, f.uv);
        return groundLighting * (1 - cloudIntensity) +
               cloudLighting * cloudIntensity;
    }

    Color getBaseColor(Vector2f uv, Vector2f uv_p) {
        return textureFilter(terrainMat->mat.diffuse.texture.value(), uv);
    }

    void GUI() {
        ImGui::ColorEdit4("Terrain base", (float*)&terrainMat->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("City lights", (float*)&terrainMat->mat.emissive.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Ocean diffuse", (float*)&oceanMat->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Ocean specular", (float*)&oceanMat->mat.specular.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Cloud diffuse", (float*)&cloudMat->mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);

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
