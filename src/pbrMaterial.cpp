#include "pbrMaterial.h"
#include "camera.h"
#include "data.h"
#include <memory>

using std::clamp, std::pow, std::max;

void PBRMaterial::GUI() {
    albedo->Gui("Albedo");
    metallic->Gui("Metallic");
    roughness->Gui("Roughness");
    ambientOcclusion->Gui("Ambient occlusion");
    Material::GUI();
}

Color fresnelSchlick(float cosTheta, Color F0) {
    return (Color(1,1,1,1) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f) + F0;
}

float DistributionGGX(Vec3 N, Vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(N.dot(H), 0.0f);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = M_PIf * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(Vec3 N, Vec3 V, Vec3 L, float roughness)
{
    float NdotV = max(N.dot(V), 0.0f);
    float NdotL = max(N.dot(L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

Color PBRMaterial::shade(Fragment &f, Color previous, Scene &scene) {
    shared_ptr<Camera> camera = currentWindow->camera;
    Color albedo = f.baseColor;
    float metallic = this->metallic->sample(f);
    float roughness = this->roughness->sample(f);
    float ao = this->ambientOcclusion->sample(f);

    Vec3 N = f.normal;
    Vec3 V = (f.worldPos - camera->obj->globalPosition).normalized();

    Color Lo{0, 0, 0, 0};
    for (size_t i = 0; i < scene.lights.size(); i++) {
        auto [radiance, L] = scene.lights[i]->sample(f.worldPos, scene);
        Vec3 H = (L + V).normalized();
        Color F0{0.04f, 0.04f, 0.04f, 1.0f};
        F0 = Color::mix(F0, albedo, metallic);
        Color F = fresnelSchlick(max(H.dot(V), 0.0f), F0);
        float NDF = DistributionGGX(N, H, roughness);       
        float G   = GeometrySmith(N, V, L, roughness);
        Color numerator    = F * NDF * G;
        float denominator = 4.0f * max(N.dot(V), 0.0f) * max(N.dot(L), 0.0f)  + 0.0001f;
        Color specular     = numerator / denominator;
        Color kS = F;
        Color kD = Color(1,1,1,1) - kS;
        kD *= 1.0f - metallic;
        float NdotL = max(N.dot(L), 0.0f);        
        Lo += (kD * albedo / M_PIf + specular) * radiance * NdotL;
    }
    Color ambient = scene.ambientLight * scene.ambientLight.a * albedo * ao;
    Color res = ambient + Lo;

    if(currentWindow->camera->whitePoint == 0) // Don't waste cycles if it won't be used
        currentWindow->camera->maximumColor = max(currentWindow->camera->maximumColor, res.luminance()); // This doesn't take transparency into account

    return res;
}