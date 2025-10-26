#ifndef __GUI_H__
#define __GUI_H__

#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h
#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads
#include "math.h"
#include "data.h"
#include "material.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace ImGui {
    template <typename T>
    bool RadioButton(const char* label, T* v, T v_button)
    {
        const bool pressed = RadioButton(label, *v == v_button);
        if (pressed)
            *v = v_button;
        return pressed;
    }
}

enum class GuiMaterialAssignMode { None, Face, Mesh };
extern shared_ptr<Material> guiSelectedMaterial;
extern GuiMaterialAssignMode guiMaterialAssignMode;
extern vector<std::weak_ptr<Material>> materials;

void guiUpdate(sf::RenderWindow &window, sf::Clock &deltaClock, shared_ptr<Scene> editingScene);
#endif /* __GUI_H__ */
