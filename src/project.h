#ifndef __PROJECT_H__
#define __PROJECT_H__
#include <SFML/System/Vector3.hpp>
#include <math.h>
#include "data.h"
#include "matrix.h"
using sf::Vector3f;
using std::sin, std::cos;

struct Projection {
    Vector3f worldPos;
    Vector3f screenPos;
    Vector3f normal;
};

void makePerspectiveProjectionMatrix();

Projection perspectiveProject(Vector3f a);

#endif /* __PROJECT_H__ */
