#include "light.h"
#include "data.h"
#include "matrix.h"
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

float Light::shadowSubsample(RenderTarget *frame, Vector2u pos, float dist) {
    float bias = currentWindow->scene->shadowBias;
    float z = frame->zBuffer[pos.x + frame->size.x * pos.y];
    return dist < z + bias ? 1 : 0;
}

float Light::shadowSample(RenderTarget *frame, Vec3 projected) {
    float dist = projected.z;
    Vector2f pos = Vector2f{projected.x + 1, projected.y + 1}
        .componentWiseMul(Vector2f{(frame->size.x-1) / 2.0f, (frame->size.y-1) / 2.0f});

    if(currentWindow->scene->bilinearShadowFiltering) {
        float decimalsX = pos.x - floor(pos.x);
        float decimalsY = pos.y - floor(pos.y);

        float s1 = shadowSubsample(frame, {(uint)floor(pos.x), (uint)floor(pos.y)}, dist);
        float s2 = shadowSubsample(frame, {(uint)floor(pos.x), (uint)ceil(pos.y)}, dist);
        float s3 = shadowSubsample(frame, {(uint)ceil(pos.x), (uint)floor(pos.y)}, dist);
        float s4 = shadowSubsample(frame, {(uint)ceil(pos.x), (uint)ceil(pos.y)}, dist);

        return lerp2d(s1, s2, s3, s4,  decimalsY, decimalsX);
    }
    else {
        return shadowSubsample(frame, {(uint)round(pos.x), (uint)round(pos.y)}, dist);
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






std::array<TransformMatrix, 6> faces = {
    makeRotationMatrix({0, M_PI_2 , M_PI}), // +x
    makeRotationMatrix({ M_PI_2, 0, M_PI}), // +y
    makeRotationMatrix({0, 0      , M_PI}), // +z
    makeRotationMatrix({0,-M_PI_2 , M_PI}), // -x
    makeRotationMatrix({-M_PI_2, 0, M_PI}), // -y
    makeRotationMatrix({0, M_PI   , M_PI}), // -z
};

void PointLight::updateShadowMap() {
    if(shadowMapCam) {
        shadowMapCam->orthographic = false; // Need it to be orthographic to get a useful projection in sample()
        shadowMapCam->fov = 90;             // Orthographic is only needed when calling project() but
                                            // Setting it with every sample is expensive so lets just
                                            // have it always be orthographic except when rendering
        TransformMatrix initialTransform = obj->transformRotation;
        for (int i = 0; i < 6; i++) {
            obj->transformRotation = faces[i] * initialTransform;
            shadowMapCam->frame = shadowMaps[i].get();
            shadowMapCam->render();
        }
        obj->transformRotation = initialTransform;
        shadowMapCam->orthographic = true; // 
        shadowMapCam->fov = 1;
        shadowMapCam->makeProjectionMatrix();
    }
}

std::pair<Color, Vec3> PointLight::sample(Vec3 pos, Scene &scene) {
    Vec3 diff = pos - obj->globalPosition;
    float distSq = diff.lengthSquared();
    float dist = std::sqrt(distSq);
    float strength = color.a;

    if(shadowMapCam) {
        Vec3 L = shadowMapCam->project(pos).screenPos;
        Vector2f uv = {0,0};
        float z = 0;
        size_t n = 0;

        if(abs(L.y) <= L.x && abs(L.z) <= L.x) { // +x
            uv = Vector2f{L.z, -L.y} / L.x;
            z = L.x;
            n = 0;
        }
        else if(abs(L.x) <= L.y && abs(L.z) <= L.y) { // +y
            uv = Vector2f{-L.x, L.z} / L.y;
            z = L.y;
            n = 4;
        }
        else if(abs(L.x) <= L.z && abs(L.y) <= L.z) { // +z
            uv = Vector2f{-L.x, -L.y} / L.z;
            z = L.z;
            n = 2;
        }
        else if(abs(L.y) <= -L.x && abs(L.z) <= -L.x) { // -x
            uv = Vector2f{L.z, L.y} / L.x;
            z = -L.x;
            n = 3;
        }
        else if(abs(L.x) <= -L.y && abs(L.z) <= -L.y) { // -y
            uv = Vector2f{L.x, L.z} / L.y;
            z = -L.y;
            n = 1;
        }
        else if(abs(L.x) <= -L.z && abs(L.y) <= -L.z) { // -z
            uv = Vector2f{-L.x, L.y} / L.z;
            z = -L.z;
            n = 5;
        }

        strength *= shadowSample(shadowMaps[n].get(), Vec3{uv.x, uv.y, z});
    }

    return {color * (strength / distSq), diff / dist};
}

void PointLight::init(Object *obj) {
    Component::init(obj);
    if(shadowMapCam)
        shadowMapCam->init(obj);
}

void PointLight::setupShadowMap(Vector2u size) {
    shadowMapCam = std::make_shared<Camera>();
    shadowMapCam->init(obj);
    shadowMapCam->shadowMap = true;
    shadowMapCam->orthographic = true;
    for (int i = 0; i < 6; i++) 
        shadowMaps[i] = std::make_shared<RenderTarget>(size, true, true);
}

void PointLight::GUI() {
    Light::GUI();
    if (shadowMapCam && ImGui::TreeNode("Shadow map")) {
        if(ImGui::DragScalarN("Resolution", ImGuiDataType_U32, &shadowMaps[0]->size.x, 2))
            for (int i = 0; i < 6; i++) 
                shadowMaps[i]->changeSize(shadowMaps[0]->size, true);
        ImGui::TreePop();
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
        Vec3 projected = shadowMapCam->project(pos).screenPos;
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
        Vec3 projected = shadowMapCam->project(pos).screenPos;
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
