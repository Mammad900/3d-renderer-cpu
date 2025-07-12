#include "phongMaterial.h"
#include "data.h"

using std::max, std::min;

Vector3f v2reflect(Vector3f in, Vector3f normal) {
    return in - normal * in.dot(normal) * 2.0f;
}

void PhongMaterial::GUI() {
    mat.diffuse->Gui("Diffuse");
    mat.specular->Gui("Specular");
    mat.tint->Gui("Tint");
    mat.emissive->Gui("Emissive");
    if(mat.normalMap)
        mat.normalMap.value()->Gui("Normal map");
    Material::GUI();
}

Color PhongMaterial::shade(Fragment &f, Color previous, Scene *scene) {
    Vector3f viewDir = (scene->camera->obj->globalPosition - f.worldPos).normalized();
    if(!flags.transparent && flags.doubleSided && f.isBackFace)
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
        auto [light, direction] = scene->lights[i]->sample(f.worldPos);

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

    if(flags.transparent) {
        if(matTint.a == 0)
            matTint = mat.tint->sample(f);
        lighting = previous * matTint + lighting;
    }

    if(scene->camera->whitePoint == 0) // Don't waste cycles if it won't be used
        scene->maximumColor = max(scene->maximumColor, lighting.luminance()); // This doesn't take transparency into account

    return lighting;
}
