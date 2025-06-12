#ifndef __FOG_H__
#define __FOG_H__

#include "data.h"

void fog() {
    float tanFOV = std::tan(scene->camera->fov * M_PI / 360);
    for (int y = 0; y < (int)frame->size.y; y++) {
        for (int x = 0; x < (int)frame->size.x; x++) {
            size_t i = x + frame->size.x * y;
            float z = frame->zBuffer[i];
            Vector2f world = Vector2f{x / (float)frame->size.x, y / (float)frame->size.y} * z * tanFOV; // Reconstruct world space X and Y
            float dist = std::sqrt(
                world.lengthSquared() + z * z
            ); // Account for X and Y, to make it radial vs flat
            float visibility = std::clamp(
                std::powf(0.5f, dist * scene->fogColor.a), 0.0f, 1.0f
            ); // Exponential falloff
            Color result = frame->framebuffer[i] * visibility +
                           scene->fogColor * (1 - visibility); // Lerp
            frame->framebuffer[i] = result;
        }
    }
}

#endif /* __FOG_H__ */
