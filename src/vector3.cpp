#include "vector3.h"

Vec3 Vec3::normalized() const  {
    assert(*this != Vec3() && "Vec3::normalized() cannot normalize a zero vector");
    return (*this) / length();
}