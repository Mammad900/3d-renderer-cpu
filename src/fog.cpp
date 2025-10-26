#include "data.h"
#include "fog.h"

Color getVisibility(Color in, float sampleLength) {
    return {
        std::expf(-sampleLength * in.r), // Exponential falloff
        std::expf(-sampleLength * in.g),
        std::expf(-sampleLength * in.b),
    };
}

Color sampleFog(Vec3 start, Vec3 end, Color background, Scene &scene, shared_ptr<Volume> volume) {
    if(!volume)
        return background;
    
    if (scene.godRays) {
        float sampleLength = scene.godRaysSampleSize;
        Color visibility = getVisibility(volume->intensity, sampleLength);
        Vec3 diff = end - start;
        Vec3 now = start;
        float remaining = diff.length();
        Vec3 step = diff * (sampleLength / remaining); // diff.normalized()
        Color color = background;
        while (remaining > 0) {
            now += step;
            Color visibilityNow = remaining > sampleLength ? visibility : getVisibility(volume->intensity, remaining);
            Color lighting = {0,0,0,1};
            for (size_t i = 0; i < scene.lights.size(); i++)
                lighting += scene.lights[i]->sample(now, scene).first;
            color = Color::mix(lighting * volume->diffuse + volume->emissive, color, visibilityNow);
            remaining-= sampleLength;
        }
        return color;
    }
    float dist = (start - end).length();
    Color visibility = getVisibility(volume->intensity, dist);
    return Color::mix(volume->diffuse + volume->emissive, background, visibility);
}
