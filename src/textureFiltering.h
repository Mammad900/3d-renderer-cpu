#ifndef __TEXTUREFILTERING_H__
#define __TEXTUREFILTERING_H__

#include <math.h>
#include "color.h"
#include <SFML/Graphics.hpp>

using sf::Vector2f;

Color textureFilter(sf::Image *texture, Vector2f pos) {
    Vector2u s = texture->getSize();
    sf::Color px = texture->getPixel({
        std::clamp((uint)round(pos.x*s.x), 0u, s.x-1),
        std::clamp((uint)round(pos.y*s.y), 0u, s.y-1) 
    });
    return {
        .r = (float)px.r / 255.0f,
        .g = (float)px.g / 255.0f,
        .b = (float)px.b / 255.0f,
        .a = (float)px.a / 255.0f
    };
}

#endif /* __TEXTUREFILTERING_H__ */
