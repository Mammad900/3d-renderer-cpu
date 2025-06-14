#include "data.h"
#include "fog.h"

void Camera::fogPixel(int x, int y, RenderTarget *frame) {
    size_t i = x + frame->size.x * y;
    float z = frame->zBuffer[i];
    if(z==INFINITY)
        return;
    Vector2f world = (Vector2f{x / (float)frame->size.x, y / (float)frame->size.y} - Vector2f{0.5,0.5}) * 2.0f * z * tanHalfFov; // Reconstruct world space X and Y
    frame->framebuffer[i] = sampleFog({0,0,0}, {world.x, world.y, z}, frame->framebuffer[i]);
}

Color sampleFog(Vector3f start, Vector3f end, Color background) {
    float dist = (start - end).length();
    float visibility = std::clamp(std::powf(0.5f, dist * scene->fogColor.a), 0.0f, 1.0f); // Exponential falloff
    return background * visibility + scene->fogColor * (1 - visibility); // Lerp
}
