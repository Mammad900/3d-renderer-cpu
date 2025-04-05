#ifndef __COLOR_H__
#define __COLOR_H__
#include <SFML/Graphics/Color.hpp>
#include <algorithm>

typedef float colorComponent_t;

struct Color {
    colorComponent_t r;
    colorComponent_t g;
    colorComponent_t b;
    colorComponent_t a;

    Color operator+(const Color &other) const {
        return {r + other.r, g + other.g, b + other.b, a + other.a};
    }
    Color operator-(const Color &other) const {
        return {r - other.r, g - other.g, b - other.b, a - other.a};
    }
    constexpr Color& operator+=(const Color& right)
    {
        r+=right.r;
        g+=right.g;
        b+=right.b;
        a+=right.a;
        return *this;
    }
    constexpr Color& operator*=(const Color& right)
    {
        r*=right.r;
        g*=right.g;
        b*=right.b;
        a*=right.a;
        return *this;
    }
    Color operator*(const Color &other) const {
        return {r * other.r, g * other.g, b * other.b, a * other.a};
    }
    Color operator/(const Color &other) const {
        return {r / other.r, g / other.g, b / other.b, a / other.a};
    }
    Color operator*(float scalar) const {
        return {r * scalar, g * scalar, b * scalar, a * scalar};
    }
    Color operator/(float scalar) const {
        return {r / scalar, g / scalar, b / scalar, a / scalar};
    }
    bool operator==(const Color &other) const {
        return (r == other.r && g == other.g && b == other.b && a == other.a);
    }
    sf::Color toSFColor() {
        // Constrain the color components to 255 if they exceed the valid range
        int r = std::clamp((int)(this->r * 255), 0, 255);
        int g = std::clamp((int)(this->g * 255), 0, 255);
        int b = std::clamp((int)(this->b * 255), 0, 255);
        return sf::Color(r, g, b);
    }
};

#endif /* __COLOR_H__ */
