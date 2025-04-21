#ifndef __BASEMATERIAL_H__
#define __BASEMATERIAL_H__

#include "object.h"
#include "textureFiltering.h"

#define COLORMAP(x) ((x).color * (((x).texture) ? textureFilter((x).texture.value(), uv) : Color{1,1,1,1} ))

class BaseMaterial : public Material {
public:
    BaseMaterialProps mat;

    BaseMaterial(BaseMaterialProps &mat, MaterialFlags flags) : mat(mat) {
        this->flags = flags;
        needsTBN = mat.normalMap.has_value();
    }

    Color getBaseColor(Vector2f uv, Vector2f uv_p) {
        return COLORMAP(mat.diffuse);
    }

    Color shade(Fragment &f, Color previous) {
        Vector2f uv = f.uv;
        Vector3f viewDir = (cam - f.position).normalized();
        if(!(flags & Transparent) && (flags & DoubleSided) && f.isBackFace)
            f.normal *= -1.0f;
        Color matSpecular = COLORMAP(mat.specular);
        float shininess = pow(2.0f, matSpecular.a * 25.5f);
        Color matEmissive = COLORMAP(mat.emissive);

        Color diffuse = ambientLight * ambientLight.a;
        Color specular = {0, 0, 0, 1};

        if(mat.normalMap) {
            Vector3f normal = ((textureFilter(mat.normalMap.value(), f.uv) * 2.0f) - Vector3f{1.0f,1.0f,1.0f}).componentWiseMul({-1,-1,1});
            normal = f.tangent*normal.x + f.bitangent*normal.y + f.normal*normal.z;
            f.normal = normal;
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
            float diffuseIntensity = f.normal.dot(direction);
            if((flags & Transparent) && (flags & DoubleSided))
                diffuseIntensity = abs(diffuseIntensity);
            if (f.baseColor.a > 0) {
                diffuse += light.color * max(diffuseIntensity, 0.0f) * intensity;
            }
            if(matSpecular.a > 0) {
                float specularIntensity = pow(max(viewDir.dot(v2reflect(direction, f.normal)), 0.0f), shininess);
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
            Color matTint = COLORMAP(mat.tint);
            lighting = previous * matTint + lighting;
        }

        if(whitePoint == 0) // Don't waste cycles if it won't be used
            maximumColor = max(maximumColor, lighting.luminance()); // This doesn't take transparency into account

        return lighting;
    }
};
#undef COLORMAP
#endif /* __BASEMATERIAL_H__ */
