#ifndef __PHONGMATERIAL_H__
#define __PHONGMATERIAL_H__

#include "imgui.h"
#include "object.h"
#include "textureFiltering.h"

Vector3f v2reflect(Vector3f in, Vector3f normal) {
    return in - normal * in.dot(normal) * 2.0f;
}

class PhongMaterial : public Material {
    using enum MaterialFlags;

public:
    PhongMaterialProps mat;

    PhongMaterial(PhongMaterialProps &mat, std::string name, MaterialFlags flags) 
        : Material(name, flags, mat.normalMap.has_value()), mat(mat) { }

    Color getBaseColor(Vector2f uv, Vector2f dUVdx, Vector2f dUVdy) {
        return mat.diffuse->sample(uv, dUVdx, dUVdy);
    }

    void GUI() {
        mat.diffuse->Gui("Diffuse");
        mat.specular->Gui("Specular");
        mat.tint->Gui("Tint");
        mat.emissive->Gui("Emissive");
        if(mat.normalMap)
            mat.normalMap.value()->Gui("Normal map");
    }

    Color shade(Fragment &f, Color previous) {
        Vector3f viewDir = (scene->cam - f.worldPos).normalized();
        if(!(flags & Transparent) && (flags & DoubleSided) && f.isBackFace)
            f.normal *= -1.0f;
        Color matSpecular = mat.specular->sample(f);
        float shininess = pow(2.0f, matSpecular.a * 25.5f);
        Color matEmissive = mat.emissive->sample(f);

        Color diffuse = scene->ambientLight * scene->ambientLight.a;
        Color sss = {0, 0, 0, 1};
        Color specular = {0, 0, 0, 1};

        Vector3f normal = f.normal;
        if (mat.normalMap) {
            normal = mat.normalMap.value()->sample(f);
            normal = f.tangent * normal.x
                   + f.bitangent*normal.y
                   + f.normal*normal.z;
            normal = normal.normalized();
        }

        for (size_t i = 0; i < scene->lights.size(); i++)
        {
            Light &light = scene->lights[i];
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

        Color matTint{0,0,0,0};
        if (!(flags & Transparent) && (flags & DoubleSided))
            matTint = mat.tint->sample(f);
        
        Color lighting = 
            diffuse * f.baseColor +
            sss * matTint +
            specular * matSpecular +
            matEmissive;

        if(flags & MaterialFlags::Transparent) {
            if(matTint.a == 0)
                matTint = mat.tint->sample(f);
            lighting = previous * matTint + lighting;
        }

        if(scene->whitePoint == 0) // Don't waste cycles if it won't be used
            scene->maximumColor = max(scene->maximumColor, lighting.luminance()); // This doesn't take transparency into account

        return lighting;
    }
};

#endif /* __PHONGMATERIAL_H__ */
