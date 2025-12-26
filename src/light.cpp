#include "light.h"
#include "data.h"
#include "object.h"
#include "textureFiltering.h"
#include <imgui.h>
#include <memory>

using std::floor, std::ceil;

float smoothstep(float edge0, float edge1, float x) {
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}

float shadowSample(RenderTarget *frame, Vec3 projected) {
    float bias = currentWindow->scene->shadowBias;
    float dist = projected.z;
    Vector2f pos = Vector2f{projected.x + 1, projected.y + 1}
        .componentWiseMul(Vector2f{frame->size.x / 2.0f, frame->size.y / 2.0f});

    if(currentWindow->scene->bilinearShadowFiltering) {
        float decimalsX = pos.x - floor(pos.x);
        float decimalsY = pos.y - floor(pos.y);

        vector<float> &zBuffer = frame->zBuffer;
        uint sizeX = frame->size.x;

        float z1 = zBuffer[(uint)floor(pos.x) + sizeX * (uint)floor(pos.y)];
        float z2 = zBuffer[(uint)floor(pos.x) + sizeX * (uint)ceil(pos.y)];
        float z3 = zBuffer[(uint)ceil(pos.x) + sizeX * (uint)floor(pos.y)];
        float z4 = zBuffer[(uint)ceil(pos.x) + sizeX * (uint)ceil(pos.y)];

        return lerp2d(
            (float)(dist < z1 + bias), 
            (float)(dist < z2 + bias), 
            (float)(dist < z3 + bias), 
            (float)(dist < z4 + bias),
            decimalsY, decimalsX
        );
    }
    else {
        float z = frame->zBuffer[(uint)round(pos.x) + frame->size.x * (uint)round(pos.y)];
        return dist < z + bias ? 1.f : 0.f;
    }
}

void Light::update() { // Try every frame to add this light to the scene. Can't be done in constructor because its not in scene tree yet.
    if(addedToScene) return;
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene) return;
    scene->lights.push_back(this);
    addedToScene = true;
}

Light::~Light() {
    if(!obj) return;
    shared_ptr<Scene> scene = obj->scene.lock();
    if(!scene) return;
    auto it = std::find(scene->lights.begin(), scene->lights.end(), this);
    if (it != scene->lights.end()) {
        scene->lights.erase(it);
    }
}






void DirectionalLight::init(Object *obj) {
    Component::init(obj);
    if(shadowMapCam)
        shadowMapCam->init(obj);
}

std::pair<Color, Vec3> DirectionalLight::sample(Vec3 pos, Scene &scene) {
    float strength = color.a;
    if(shadowMapCam) {
        Vec3 projected = shadowMapCam->perspectiveProject(pos).screenPos;
        if(projected.z < 0 || projected.x < -1 || projected.x > 1 || projected.y < -1 || projected.y > 1 )
            strength = litOutsideShadowMap;
        else {
            strength *= shadowSample(shadowMapCam->frame, projected);
        }
    }
    return {color * strength, direction};
}

void DirectionalLight::setupShadowMap(Vector2u size, float fov) {
    shadowMapCam = std::make_shared<Camera>();
    shadowMapCam->init(obj);
    shadowMapCam->shadowMap = true;
    shadowMapCam->orthographic = true;
    shadowMapCam->fov = fov;
    shadowMap = std::make_shared<RenderTarget>(size, true, true);
    shadowMapCam->frame = shadowMap.get();
}

void DirectionalLight::updateShadowMap() {
    if(shadowMapCam) {
        shadowMapCam->render();
    }
}

void DirectionalLight::GUI() {
    Light::GUI();
    if (shadowMapCam && ImGui::TreeNode("Shadow map")) {
        ImGui::DragFloat("FOV", &shadowMapCam->fov, 1, 0.1, 100, "%.3f", ImGuiSliderFlags_Logarithmic);
        if(ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &shadowMapCam->frame->size.x, 2))
            shadowMapCam->frame->changeSize(shadowMapCam->frame->size, true);
        ImGui::TreePop();
    }
}






void SpotLight::init(Object *obj) {
    Component::init(obj);
    if(shadowMapCam)
        shadowMapCam->init(obj);
}

void SpotLight::update() {
    Light::update();
    spreadInnerCos = std::cos(spreadInner);
    spreadOuterCos = std::cos(spreadOuter);
    if(spreadInnerCos < spreadOuterCos)
        std::swap(spreadInnerCos, spreadOuterCos);
    direction = Vec3{0, 0, 1} * obj->transformRotation;
}

void SpotLight::updateShadowMap() {
    if(shadowMapCam) {
        shadowMapCam->fov = std::max(spreadOuter, spreadInner) * (360.0f / M_PIf);
        shadowMapCam->render();
    }
}

std::pair<Color, Vec3> SpotLight::sample(Vec3 pos, Scene &scene) {

    Vec3 diff = pos - obj->globalPosition;
    float distSq = diff.lengthSquared();
    float dist = std::sqrt(distSq);
    Vec3 distNormalized = diff / dist;
    float cos = distNormalized.dot(direction);
    if(cos < spreadOuterCos)
        return {{0, 0, 0, 0}, {0, 0, 0}};
    float strength = smoothstep(spreadOuterCos, spreadInnerCos, cos);


    if(shadowMapCam) {
        Vec3 projected = shadowMapCam->perspectiveProject(pos).screenPos;
        if(projected.z < 0 || projected.x < -1 || projected.x > 1 || projected.y < -1 || projected.y > 1 )
            strength = 0;
        else {
            strength *= shadowSample(shadowMapCam->frame, projected);
        }
    }
    return {color * (color.a * strength / distSq), distNormalized};
}

void Light::GUI() {
    ImGui::ColorEdit4("Color", (float*)&color, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
}

void SpotLight::setupShadowMap(Vector2u size) {
    shadowMapCam = std::make_shared<Camera>();
    shadowMapCam->init(obj);
    shadowMapCam->shadowMap = true;
    shadowMap = std::make_shared<RenderTarget>(size, true, true);
    shadowMapCam->frame = shadowMap.get();
}

void SpotLight::GUI() {
    Light::GUI();
    ImGui::SliderFloat("Spread inner", &spreadInner, 0, M_PI_2);
    ImGui::SliderFloat("Spread outer", &spreadOuter, 0, M_PI_2);
    if (shadowMapCam && ImGui::TreeNode("Shadow map")) {
        if(ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &shadowMapCam->frame->size.x, 2))
            shadowMapCam->frame->changeSize(shadowMapCam->frame->size, true);
        ImGui::TreePop();
    }
}
