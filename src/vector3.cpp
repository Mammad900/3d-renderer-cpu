#include "vector3.h"

Vec3 Vec3::normalized() const  {
    assert(*this != Vec3() && "Vec3::normalized() cannot normalize a zero vector");
    return (*this) / length();
}

Vec3 Vec3::normalizedSafe() const  {
    float l = length();
    if(l == 0) return {0,0,0};
    return (*this) / l;
}