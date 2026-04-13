#ifndef __AABB_H__
#define __AABB_H__

#include "vector3.h"

struct AABB {
    Vec3 min, max;

    AABB(Vec3 min, Vec3 max) : min(min), max(max) {}
    AABB(float nx, float ny, float nz, float px, float py, float pz) : min(nx, ny, nz), max(px, py, pz) {}
    static AABB fromCenter(Vec3 center, Vec3 size) { return AABB(center - size/2.f, center + size/2.f); }

    Vec3 center() { return 0.5f * (min+max); }
    Vec3 size() { return max - min; }
};

#endif /* __AABB_H__ */
