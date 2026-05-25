#ifndef __AABB_H__
#define __AABB_H__

// #include "matrix.h"
#include "vector3.h"

struct AABB {
    Vec3 min, max;

    AABB() = default;
    AABB(Vec3 min, Vec3 max) : min(min), max(max) {}
    AABB(float nx, float ny, float nz, float px, float py, float pz) : min(nx, ny, nz), max(px, py, pz) {}
    static AABB fromCenter(Vec3 center, Vec3 size) { return AABB(center - size/2.f, center + size/2.f); }

    Vec3 center() { return 0.5f * (min+max); }
    Vec3 size() { return max - min; }

    // AABB operator *(TransformMatrix transform) {
    //     // Taken from https://gist.github.com/cmf028/81e8d3907035640ee0e3fdd69ada543f
    //     // transform to center/extents box representation
    //     Vec3 center = this->center();
    //     Vec3 extents = max - center;

    //     // transform center
    //     Vec3 t_center = center * transform;

    //     // transform extents (take maximum)
    //     mat3 abs_mat = mat3(abs(m[0].xyz), abs(m[1].xyz), abs(m[2].xyz));
    //     vec3 t_extents = abs_mat * extents;

    //     // transform to min/max box representation
    //     vec3 tmin = t_center - t_extents;
    //     vec3 tmax = t_center + t_extents;
        
    //     AABB rbox;
        
    //     rbox.min = tmin;
    //     rbox.max = tmax;
        
    //     return rbox;
    // }
};

#endif /* __AABB_H__ */
