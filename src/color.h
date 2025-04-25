#ifndef __COLOR_H__
#define __COLOR_H__
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector3.hpp>
#include <algorithm>
#include <math.h>
#include <istream>

typedef float colorComponent_t;
using sf::Vector3f;

struct Color {
    colorComponent_t r;
    colorComponent_t g;
    colorComponent_t b;
    colorComponent_t a;

    static Color fromSFColor(sf::Color c) {
        return {
            (float)c.r / 255.0f,
            (float)c.g / 255.0f,
            (float)c.b / 255.0f,
            (float)c.a / 255.0f
        };
    }

    Color operator+(const Color &other) const {
        return {r + other.r, g + other.g, b + other.b, a + other.a};
    }
    Color operator-(const Color &other) const {
        return {r - other.r, g - other.g, b - other.b, a - other.a};
    }
    constexpr Color &operator+=(const Color &right) {
        r += right.r;
        g += right.g;
        b += right.b;
        a += right.a;
        return *this;
    }
    constexpr Color &operator*=(const Color &right) {
        r *= right.r;
        g *= right.g;
        b *= right.b;
        a *= right.a;
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
    Color operator+(float scalar) const {
        return {r + scalar, g + scalar, b + scalar, a + scalar};
    }
    Color operator-(float scalar) const {
        return {r - scalar, g - scalar, b - scalar, a - scalar};
    }
    bool operator==(const Color &other) const {
        return (r == other.r && g == other.g && b == other.b && a == other.a);
    }
    operator sf::Color() const {
        // Constrain the color components to 255 if they exceed the valid range
        int r = std::clamp((int)(this->r * 255), 0, 255);
        int g = std::clamp((int)(this->g * 255), 0, 255);
        int b = std::clamp((int)(this->b * 255), 0, 255);
        return sf::Color(r, g, b);
    }
    operator Vector3f() const { return Vector3f{r, g, b}; }

    friend std::istream& operator>>(std::istream &is, Color &color) {
        is >> color.r >> color.g >> color.b >> color.a;
        return is;
    }

    float luminance() { return 0.2126f * r + 0.7152f * g + 0.0722f * b; }
    Color changeLuminance(float lOut) { return (*this) * (lOut / luminance()); }
    Color reinhardtTonemap(float maximumColor) {
        float lOld = luminance();
        float numerator = lOld * (1.0f + (lOld / (maximumColor*maximumColor)));
        float lNew = numerator / (1.0f + lOld);
        Color newColor = changeLuminance(lNew);
        return newColor;
    }
};

#endif /* __COLOR_H__ */
