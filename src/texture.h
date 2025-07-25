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
            ImGui::SliderFloat(label.c_str(), &value, 0, 1);
        else if constexpr(std::is_same_v<T, Vec3>)
            ImGui::DragFloat3(label.c_str(), (float*)&value);
    }
};

template <typename T>
class ErrorTexture : public Texture<T> {
public:
    ErrorTexture() {}
    T sample(Vector2f uv, Vector2f, Vector2f) {
        uv *= 8.0f;
        if((int(uv.x)%2 == 0) ^ (int(uv.y)%2 == 0)) {
            if constexpr(std::is_same_v<T, Color>)
                return Color{0, 0, 0, 1}; // Black
            else if constexpr(std::is_same_v<T, float>)
                return 1.0f;
            else if constexpr(std::is_same_v<T, Vec3>)
                return Vec3{-1, -1, 1};
        } else {
            if constexpr(std::is_same_v<T, Color>)
                return Color{1, 0, 1, 1}; // Magenta
            else if constexpr(std::is_same_v<T, float>)
                return 0.0f;
            else if constexpr(std::is_same_v<T, Vec3>)
                return Vec3{1, 1, 1};
        }
    }
};

#endif /* __TEXTURE_H__ */
