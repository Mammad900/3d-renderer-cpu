#include "phongMaterial.h"
#include "camera.h"
#include "data.h"
#include "imgui.h"
#include <memory>

using std::max, std::min;

Vec3 v2reflect(Vec3 in, Vec3 normal) {
    return in - normal * in.dot(normal) * 2.0f;
}

void PhongMaterial::GUI() {
    mat.diffuse->Gui("Diffuse");
    mat.specular->Gui("Specular");
    mat.tint->Gui("Tint");
    mat.emissive->Gui("Emissive");
    ImGui::ColorEdit4("Environment reflection", (float*)&mat.environmentReflection, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);

    if(mat.normalMap)
        mat.normalMap->Gui("Normal map");
    Material::GUI();
}

Color PhongMaterial::shade(Fragment &f, Color previous, Scene &scene) {
    shared_ptr<Camera> camera = currentWindow->camera;
    Vec3 viewDir = (camera->obj->globalPosition - f.worldPos).normalized();
    if(!flags.transparent && flags.doubleSided && f.isBackFace)
        f.normal *= -1.0f;
    Color matSpecular = mat.specular->sample(f);
    float shininess = pow(2.0f, matSpecular.a * 25.5f);
    Color matEmissive = mat.emissive->sample(f);

    Color diffuse = scene.ambientLight * scene.ambientLight.a;
    Color sss = {0, 0, 0, 1};
    Color specular = {0, 0, 0, 1};

    Vec3 normal = f.normal;
    if (mat.normalMap) {
        normal = mat.normalMap->sample(f);
        normal = f.tangent * normal.x
                + f.bitangent*normal.y
                + f.normal*normal.z;
        normal = normal.normalized();
    }

    for (size_t i = 0; i < scene.lights.size(); i++)
    {
        auto [light, direction] = scene.lights[i]->sample(f.worldPos, scene);

        if(light.a == 0) continue; // No light received, don't bother calculating

        float receivedLight = normal.dot(direction);

        // Transparent double sided objects can be lit from any side.
        if(flags.transparent && flags.doubleSided)
            receivedLight = abs(receivedLight);

        // Diffuse
        if(receivedLight > 0) {
            if (f.baseColor.a > 0)
                diffuse += light * max(receivedLight, 0.0f);
        }
        // Or subsurface scattering
        else if(!flags.transparent && flags.doubleSided) {
            sss += light * max(-receivedLight, 0.0f);
        }

        // Specular highlights
        if(matSpecular.a > 0) {
            float specularIntensity = pow(max(viewDir.dot(v2reflect(direction, normal)), 0.0f), shininess);
            if(receivedLight <= 0)
                specularIntensity = 0;
            specular += light * specularIntensity;
        }
    }

    Color matTint{0,0,0,0};
    if (!flags.transparent && flags.doubleSided)
        matTint = mat.tint->sample(f);
    
    Color lighting = 
        diffuse * f.baseColor +
        sss * matTint +
        specular * matSpecular +
        matEmissive;

    if (mat.environmentReflection.a > 0) {
        Vec3 R = -v2reflect(viewDir, normal);
        lighting += mat.environmentReflection * scene.skyBox->sample(R);
    }

    if(flags.transparent) {
        if(matTint.a == 0)
            matTint = mat.tint->sample(f);
        lighting = previous * matTint + lighting;
    }

    if(currentWindow->camera->whitePoint == 0) // Don't waste cycles if it won't be used
        currentWindow->camera->maximumColor = max(currentWindow->camera->maximumColor, lighting.luminance()); // This doesn't take transparency into account

    return lighting;
}
