#include "light.h"
#include "data.h"
#include "textureFiltering.h"
#include <imgui.h>

using std::floor, std::ceil;

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
    auto it = std::find(obj->scene->lights.begin(), obj->scene->lights.end(), this);
    if (it != obj->scene->lights.end()) {
        obj->scene->lights.erase(it);
    }
}

std::pair<Color, Vector3f> SpotLight::sample(Vector3f pos) {
    float bias = obj->scene->shadowBias;
    Vector3f diff = pos - obj->globalPosition;
    float distSq = diff.lengthSquared();
    float dist = std::sqrt(distSq);
    Vector3f distNormalized = diff / dist;
    float cos = distNormalized.dot(direction);
    if(cos < spreadOuterCos)
        return {{0, 0, 0, 0}, {0, 0, 0}};
    float strength = smoothstep(spreadOuterCos, spreadInnerCos, cos);


    if(shadowMap) {
        Vector3f projected = shadowMap->perspectiveProject(pos).screenPos;
        if(projected.z < 0 || projected.x < -1 || projected.x > 1 || projected.y < -1 || projected.y > 1 )
            strength = 0;
        else {
            dist = projected.z;
            Vector2f pos = Vector2f{projected.x + 1, projected.y + 1}
                .componentWiseMul(Vector2f{shadowMap->tFrame->size.x / 2.0f, shadowMap->tFrame->size.y / 2.0f});

            if(obj->scene->bilinearShadowFiltering) {
                float decimalsX = pos.x - floor(pos.x);
                float decimalsY = pos.y - floor(pos.y);

                float *zBuffer = shadowMap->tFrame->zBuffer;
                uint sizeX = shadowMap->tFrame->size.x;

                float z1 = zBuffer[(uint)floor(pos.x) + sizeX * (uint)floor(pos.y)];
                float z2 = zBuffer[(uint)floor(pos.x) + sizeX * (uint)ceil(pos.y)];
                float z3 = zBuffer[(uint)ceil(pos.x) + sizeX * (uint)floor(pos.y)];
                float z4 = zBuffer[(uint)ceil(pos.x) + sizeX * (uint)ceil(pos.y)];

                strength *= lerp2d(
                    (float)(dist < z1 + bias), 
                    (float)(dist < z2 + bias), 
                    (float)(dist < z3 + bias), 
                    (float)(dist < z4 + bias),
                    decimalsY, decimalsX
                );
            }
            else {
                float z = shadowMap->tFrame->zBuffer[(uint)round(pos.x) + shadowMap->tFrame->size.x * (uint)round(pos.y)];
                if (dist > z + bias)
                    strength = 0;
            }
        }
    }
    return {color * (color.a * strength / distSq), distNormalized};
}

void Light::GUI() {
    ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
}

void SpotLight::setupShadowMap(Vector2u size) {
    shadowMap = new Camera(obj);
    shadowMap->shadowMap = true;
    shadowMap->tFrame = new RenderTarget(size, true);
}

void SpotLight::GUI() {
    Light::GUI();
    ImGui::SliderFloat("Spread inner", &spreadInner, 0, M_PI_2);
    ImGui::SliderFloat("Spread outer", &spreadOuter, 0, M_PI_2);
    if (shadowMap && ImGui::TreeNode("Shadow map")) {
        if(ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &shadowMap->tFrame->size.x, 2))
            shadowMap->tFrame->changeSize(shadowMap->tFrame->size, true);
        ImGui::TreePop();
    }
}
