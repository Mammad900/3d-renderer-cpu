#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "imgui.h"
#include "object.h"
#include "textureFiltering.h"

#define COLORMAP(x) ((x).color * (((x).texture) ? textureSample((x).texture.value(), uv, dUVdx, dUVdy) : Color{1,1,1,1} ))

class PhongMaterial : public Material {
public:
    PhongMaterialProps mat;

    PhongMaterial(PhongMaterialProps &mat, std::string name, MaterialFlags flags) 
        : Material(name, flags, mat.normalMap.has_value()), mat(mat) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return COLORMAP(mat.diffuse);
    }

    void GUI() {
        ImGui::ColorEdit4("Diffuse", (float*)&mat.diffuse.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Specular", (float*)&mat.specular.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit4("Tint", (float*)&mat.tint.color, ImGuiColorEditFlags_Float);
        ImGui::ColorEdit4("Emissive", (float*)&mat.emissive.color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        ImGui::SliderFloat("Normal Map Strength", &mat.normalMapStrength, 0, 1.5f);
    }

    Color shade(Fragment &f, Color previous, Color matSpecular, Color matEmissive, Color matTint) {
        Vector2f uv = f.uv, dUVdx = f.dUVdx, dUVdy = f.dUVdy;
        Vector3f viewDir = (cam - f.worldPos).normalized();
        if(!(flags & Transparent) && (flags & DoubleSided) && f.isBackFace)
            f.normal *= -1.0f;
        if(matSpecular.a == 0)
            matSpecular = COLORMAP(mat.specular);
        float shininess = pow(2.0f, matSpecular.a * 25.5f);
        if(matEmissive.a == 0)
            matEmissive = COLORMAP(mat.emissive);

        Color diffuse = ambientLight * ambientLight.a;
        Color sss = {0, 0, 0, 1};
        Color specular = {0, 0, 0, 1};

        Vector3f normal = f.normal;
        if (mat.normalMap) {
            normal = ((textureSample(mat.normalMap.value(), uv, dUVdx, dUVdy) * 2.0f) - Vector3f{1.0f,1.0f,1.0f}).componentWiseMul({-1,-1,1});
            normal = f.tangent * normal.x * mat.normalMapStrength
                   + f.bitangent*normal.y * mat.normalMapStrength
                   + f.normal*normal.z;
            normal = normal.normalized();
        }

        for (size_t i = 0; i < lights.size(); i++)
        {
            Light &light = lights[i];
            Vector3f direction = light.direction;
            float intensity = light.color.a;
            if(light.isPointLight) {
                Vector3f d = light.direction - f.worldPos; //direction is world pos in this case
                float l2 = d.lengthSquared();
                direction = d / std::sqrtf(l2);
                intensity /= l2;
            }

            float receivedLight = normal.dot(direction);

            // Transparent double sided objects can be lit from any side.
            if((flags & Transparent) && (flags & DoubleSided))
                receivedLight = abs(receivedLight);

            // Diffuse
            if(receivedLight > 0) {
                if (f.baseColor.a > 0)
                    diffuse += light.color * max(receivedLight, 0.0f) * intensity;
            }
            // Or subsurface scattering
            else if(!(flags & Transparent) && (flags & DoubleSided)) {
                sss += light.color * max(-receivedLight, 0.0f) * intensity;
            }

            // Specular highlights
            if(matSpecular.a > 0) {
                float specularIntensity = pow(max(viewDir.dot(v2reflect(direction, normal)), 0.0f), shininess);
                if(receivedLight <= 0)
                    specularIntensity = 0;
                specular += light.color * specularIntensity * intensity;
            }
        }
        
        if(!(flags & Transparent) && (flags & DoubleSided) && matTint.a == 0)
            matTint = COLORMAP(mat.tint);
        
        Color lighting = 
            diffuse * f.baseColor +
            sss * matTint +
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
