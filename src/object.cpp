#include "object.h"
#include <imgui.h>
#include "data.h"

void Object::update() {
    for (auto &&c : components)
        c->preUpdate();

    TransformMatrix scaleT{
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1,
    }, translate{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        position.x, position.y, position.z, 1,
    };
    myTransformRotation = scaleT * makeRotationMatrix(rotation);
    myTransform = myTransformRotation * translate;
    if (parent) {
        transform = myTransform * parent->transform;
        transformRotation = myTransformRotation * parent->transformRotation;
        globalPosition = {transform[12], transform[13], transform[14]};
    }
    else {
        transform = myTransform;
        transformRotation = myTransformRotation;
        globalPosition = position;
    }

    for (auto &&c : components)
        c->update();
    for (auto &&c : children)
        c->update();
}

void Object::GUI() {
    if(ImGui::TreeNode(name.c_str())) {
        ImGui::SliderFloat3("Rotation", (float *)&rotation, -M_PI, M_PI);
        ImGui::DragFloat3("Position", (float *)&position, 0.2f);
        ImGui::DragFloat3("Scale", (float *)&scale, 0.1f);

        ImGui::Text("Components:");
        for (size_t i = 0; i < components.size(); i++) {
            ImGui::PushID(i);
            Component *c = components[i];
            if (ImGui::TreeNode(c->name().c_str())) {
                c->GUI();
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::Text("Children:");
        for (size_t i = 0; i < children.size(); i++) {
            ImGui::PushID(i);
            children[i]->GUI();
            ImGui::PopID();
        }

        ImGui::TreePop();
    }
}

void RotatorComponent::preUpdate() { if(enable) obj->rotation += rotatePerSecond * timing.deltaTime; }

void RotatorComponent::GUI() {
    ImGui::Checkbox("Enable", &enable);
    ImGui::DragFloat3("Rotation per second", &rotatePerSecond.x, 0.05f);
}

void KeyboardControlComponent::GUI() {
    ImGui::DragFloat3("Speed", &speed.x, 0.05f);
}
