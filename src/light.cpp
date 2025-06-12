#include "light.h"
#include "data.h"
#include <imgui.h>

float smoothstep(float edge0, float edge1, float x) {
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}

Light::Light(Object *obj, Color color) : Component(obj), color(color) {
    obj->scene->lights.push_back(this);
}

Light::~Light() {
    auto it = std::find(scene->lights.begin(), scene->lights.end(), this);
    if (it != scene->lights.end()) {
        scene->lights.erase(it);
    }
}

std::pair<Color, Vector3f> SpotLight::sample(Vector3f pos) {
    Vector3f dist = pos - obj->globalPosition;
    float distSq = dist.lengthSquared();
    Vector3f distNormalized = dist / std::sqrt(distSq);
    float cos = distNormalized.dot(direction);
    if(cos < spreadOuterCos)
        return {{0, 0, 0, 0}, {0, 0, 0}};
    float strength = smoothstep(spreadOuterCos, spreadInnerCos, cos);
    return {color * (color.a * strength / distSq), distNormalized};
}

void Light::GUI() {
    ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
}

void SpotLight::GUI() {
    Light::GUI();
    ImGui::SliderFloat("Spread inner", &spreadInner, 0, M_PI_2);
    ImGui::SliderFloat("Spread outer", &spreadOuter, 0, M_PI_2);
}
