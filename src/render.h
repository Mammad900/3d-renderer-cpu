#ifndef __RENDER_H__
#define __RENDER_H__
#include "triangle.h"
#include "object.h"
#include "color.h"
#include "data.h"
#include "matrix.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using sf::Vector3f, sf::Vector2f;
using std::max;


void fog();

// void render(Scene *scene, RenderTarget *frame) {

// #pragma region // ===== FOG =====
//     if(scene->fogColor.a > 0)
//         fog();

// #pragma endregion
// }

#endif /* __RENDER_H__ */
