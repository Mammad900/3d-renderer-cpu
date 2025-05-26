#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "color.h"
#include "project.h"
#include "textureFiltering.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::max, std::abs;

struct Triangle {
    Projection s1, s2, s3;
    Vector2f uv1, uv2, uv3;
    Material *mat;
    bool cull;
};

size_t frameBufferIndex(Vector2i pos);
void drawTriangle(RenderTarget *frame, Triangle tri, bool defer);
#endif /* __TRIANGLE_H__ */
