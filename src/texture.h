#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include "color.h"
#include <SFML/System/Vector2.hpp>
#include <imgui.h>

struct Fragment;
using sf::Vector2f, sf::Vector2u;

template <typename T>
class Texture {
public:
    virtual T sample(Vector2f UV, Vector2f dUVdX, Vector2f dUVdY) = 0;
    T sample(Fragment &f);
    virtual void Gui(std::string label) {};
    virtual ~Texture() = default;
};

template <typename T>
class SolidTexture : public Texture<T> {
public:
    T value;
    SolidTexture(T value) : value(value) {}
    T sample(Vector2f, Vector2f, Vector2f) { return value; }
    void Gui(std::string label) {
        if constexpr(std::is_same_v<T, Color>)
            ImGui::ColorEdit4(label.c_str(), (float*)&value, ImGuiColorEditFlags_Float|ImGuiColorEditFlags_HDR);
        else if constexpr(std::is_same_v<T, float>)
            ImGui::DragFloat(label.c_str(), &value);
        else if constexpr(std::is_same_v<T, Vector3f>)
            ImGui::DragFloat3(label.c_str(), (float*)&value);
    }
};

#endif /* __TEXTURE_H__ */
