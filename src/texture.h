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

/// @brief Returns a * sin(bx+ct+d) + e
class SineWaveTexture : public Texture<float> {
  public:
    float a, b, c, d, e;
    bool orientation;
    SineWaveTexture(float a, float b, float c, float d, float e, bool orientation)
        : a(a), b(b), c(c), d(d), e(e), orientation(orientation) {}
    float sample(Vector2f uv, Vector2f, Vector2f);
    void Gui(std::string label);
};

enum class BlendMode {
    AlphaMix,
    Add,
    Multiply,
    Subtract
};

template<typename T, typename P>
class BlendTexture : public Texture<T> {
  public:
    Texture<T> *a;
    Texture<P> *b;


    BlendMode mode;

    BlendTexture(Texture<T> *a, Texture<P> *b, BlendMode mode) : a(a), b(b), mode(mode) {}

    T sample(Vector2f uv, Vector2f dUVdX, Vector2f dUVdY) {
        T sampleA = a->sample(uv, dUVdX, dUVdY);
        P sampleB = b->sample(uv, dUVdX, dUVdY);
        if constexpr (std::is_same_v<P, Color>) {
            if(mode == BlendMode::AlphaMix)
                return sampleB * sampleB.a + sampleA * (1 - sampleB.a);
        }
        switch (mode)
        {
        case BlendMode::Add:
            return sampleA + sampleB;
        case BlendMode::Multiply:
            return sampleA * sampleB;
        case BlendMode::Subtract:
            return sampleA - sampleB;

        default:
            throw std::runtime_error("Invalid blend mode");
        }
    }

    void Gui(std::string label) {
        if(ImGui::TreeNode(label.c_str())) {
            a->Gui("A");
            b->Gui("B");
            ImGui::TreePop();
        }
    }
};

#endif /* __TEXTURE_H__ */
