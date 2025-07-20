#include "data.h"
#include "fog.h"

void Camera::fogPixel(int x, int y) {
    size_t i = x + tFrame->size.x * y;
    float z = tFrame->zBuffer[i];

    if(z == INFINITY) {
        if(obj->scene->godRays)
            z = farClip;
        else
            return;
    }

    // Reconstruct world space X and Y
    Vector2f worldPos{x / (float)tFrame->size.x, y / (float)tFrame->size.y};
    worldPos = (Vector2f{0.5, 0.5} - worldPos) * 2.0f * z * tanHalfFov; 

    tFrame->framebuffer[i] = sampleFog(
        Vec3{worldPos.x, worldPos.y, z} * obj->transform, 
        obj->globalPosition,
        tFrame->framebuffer[i],
        obj->scene
    );
}

Color sampleFog(Vec3 start, Vec3 end, Color background, Scene *scene) {
    if(scene->godRays) {
        float sampleLength = scene->godRaysSampleSize;
        float visibility = std::clamp(std::powf(0.5f, sampleLength * scene->fogColor.a), 0.0f, 1.0f); // Exponential falloff
        Vec3 diff = end - start;
        Vec3 now = start;
        float remaining = diff.length();
        Vec3 step = diff * (sampleLength / remaining); // diff.normalized()
        Color color = background;
        while (remaining > 0) {
            now += step;
            float visibilityNow = remaining > sampleLength ? visibility : std::clamp(std::powf(0.5f, remaining * scene->fogColor.a), 0.0f, 1.0f);
            Color lighting = {0,0,0,1};
            for (size_t i = 0; i < scene->lights.size(); i++)
                lighting += scene->lights[i]->sample(now).first;
            color = color * visibilityNow + scene->fogColor * lighting * (1 - visibilityNow);
            remaining-= sampleLength;
        }
        return color;
    }
    float dist = (start - end).length();
    float visibility = std::clamp(std::powf(0.5f, dist * scene->fogColor.a), 0.0f, 1.0f); // Exponential falloff
    return background * visibility + scene->fogColor * (1 - visibility); // Lerp
}
