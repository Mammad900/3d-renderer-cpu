#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__
#include "color.h"
#include "camera.h"
#include "textureFiltering.h"
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>

using sf::Vector2f, sf::Vector2u, sf::Vector2i;
using std::swap, std::max, std::abs;

void drawTriangle(Camera *camera, Triangle tri, bool defer);
#endif /* __TRIANGLE_H__ */
