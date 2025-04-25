#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "imgui.h"
#include "object.h"
#include "textureFiltering.h"

#define COLORMAP(x) ((x).color * (((x).texture) ? textureFilter((x).texture.value(), uv) : Color{1,1,1,1} ))

class PhongMaterial : public Material {
public:
    PhongMaterialProps mat;

    PhongMaterial(PhongMaterialProps &mat, MaterialFlags flags) : mat(mat) {
        this->flags = flags;
        needsTBN = mat.normalMap.has_value();
    }

    Color getBaseColor(Vector2f uv, Vector2f uv_p) {
        return COLORMAP(mat.diffuse);
    }

    void GUI() {
        ImGui::ColorEdit4("Diffuse", (float*)&mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Specular", (float*)&mat.specular.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Tint", (float*)&mat.tint.color, ImGuiColorEditFlags_Float);
        ImGui::ColorEdit4("Emissive", (float*)&mat.emissive.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
    }

    Color shade(Fragment &f, Color previous, Color matSpecular, Color matEmissive, Color matTint) {
        Vector2f uv = f.uv;
        Vector3f viewDir = (cam - f.position).normalized();
        if(!(flags & Transparent) && (flags & DoubleSided) && f.isBackFace)
            f.normal *= -1.0f;
        if(matSpecular.a == 0)
            matSpecular = COLORMAP(mat.specular);
        float shininess = pow(2.0f, matSpecular.a * 25.5f);
        if(matEmissive.a == 0)
            matEmissive = COLORMAP(mat.emissive);

        Color diffuse = ambientLight * ambientLight.a;
        Color specular = {0, 0, 0, 1};

        Vector3f normal = f.normal;
        if (mat.normalMap) {
            normal = ((textureFilter(mat.normalMap.value(), f.uv) * 2.0f) - Vector3f{1.0f,1.0f,1.0f}).componentWiseMul({-1,-1,1});
            normal = f.tangent*normal.x + f.bitangent*normal.y + f.normal*normal.z;
        }

        for (size_t i = 0; i < lights.size(); i++)
        {
            Light &light = lights[i];
            Vector3f direction = light.direction;
            float intensity = light.color.a;
            if(light.isPointLight) {
                Vector3f d = light.direction - f.position; //direction is world pos in this case
                float l2 = d.lengthSquared();
                direction = d / std::sqrtf(l2);
                intensity /= l2;
            }
            float diffuseIntensity = normal.dot(direction);
            if((flags & Transparent) && (flags & DoubleSided))
                diffuseIntensity = abs(diffuseIntensity);
            if (f.baseColor.a > 0) {
                diffuse += light.color * max(diffuseIntensity, 0.0f) * intensity;
            }
            if(matSpecular.a > 0) {
                float specularIntensity = pow(max(viewDir.dot(v2reflect(direction, normal)), 0.0f), shininess);
                if(diffuseIntensity <= 0)
                    specularIntensity = 0;
                specular += light.color * specularIntensity * intensity;
            }
        }
        
        Color lighting = 
            diffuse * f.baseColor +
            specular * matSpecular +
            matEmissive;

        if(flags & MaterialFlags::Transparent) {
            if(matTint.a == 0)
                matTint = COLORMAP(mat.tint);
            lighting = previous * matTint + lighting;
        }

        if(whitePoint == 0) // Don't waste cycles if it won't be used
            maximumColor = max(maximumColor, lighting.luminance()); // This doesn't take transparency into account

        return lighting;
    }
    Color shade(Fragment &f, Color previous) {
        return shade(f, previous, {}, {}, {});
    }
};

#endif /* __PHONGMATERIAL_H__ */
