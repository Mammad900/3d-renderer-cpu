#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include "color.h"
#include <SFML/System/Vector2.hpp>
#include <imgui.h>
#include <memory>

struct Fragment;
using sf::Vector2f, sf::Vector2u, std::shared_ptr;

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
    shared_ptr<Texture<T>> a;
    shared_ptr<Texture<P>> b;


    BlendMode mode;

    BlendTexture(shared_ptr<Texture<T>> a, shared_ptr<Texture<P>> b, BlendMode mode) : a(a), b(b), mode(mode) {}

    T sample(Vector2f uv, Vector2f dUVdX, Vector2f dUVdY) {
        T sampleA = a->sample(uv, dUVdX, dUVdY);
        P sampleB = b->sample(uv, dUVdX, dUVdY);

        if constexpr (std::is_same_v<P, Color>) {
            if (mode == BlendMode::AlphaMix)
                return sampleB * sampleB.a + sampleA * (1 - sampleB.a);
        }

        switch (mode)
        {
        case BlendMode::Add:
            if constexpr (std::is_same_v<T, P>)
                return sampleA + sampleB;
            else
                throw std::runtime_error("Incompatible types for Add");
        case BlendMode::Multiply:
            return sampleA * sampleB;
        case BlendMode::Subtract:
            if constexpr (std::is_same_v<T, P>)
                return sampleA - sampleB;
            else
                throw std::runtime_error("Incompatible types for Subtract");
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

template<typename T>
class SliceTexture : public Texture<T> {
  public:
    Vector2f scale = {0, 0};
    Vector2f offset = {0, 0};
    shared_ptr<Texture<T>> texture;
    SliceTexture(shared_ptr<Texture<T>> texture, Vector2f scale, Vector2f offset) : texture(texture), offset(offset), scale(scale) {}

    T sample(Vector2f uv, Vector2f dUVdX, Vector2f dUVdY) {
        return texture->sample(
            uv.componentWiseMul(scale) + offset, 
            dUVdX.componentWiseMul(scale),
            dUVdY.componentWiseMul(scale)
        );
    }

    void Gui(std::string label) {
        if(ImGui::TreeNode(label.c_str())) {
            ImGui::SliderFloat2("Offset", &offset.x, 0, 1);
            ImGui::SliderFloat2("Scale", &scale.x, 0, 1);
            texture->Gui("Source");
            ImGui::TreePop();
        }
    }
};

#endif /* __TEXTURE_H__ */
