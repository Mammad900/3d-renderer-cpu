#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "object.h"
#include "camera.h"

class Light : public Component {
  public:
    Color color;

    Light(Object *obj, Color color);
    virtual ~Light();
    virtual std::pair<Color, Vector3f> sample(Vector3f pos) = 0;

    void GUI();
};

class PointLight : public Light {
  public:
    PointLight(Object *obj, Color color)
        : Light(obj, color) {}

    std::string name() { return "Point Light"; }

    std::pair<Color, Vector3f> sample(Vector3f pos) {
        Vector3f dist = pos - obj->globalPosition;
        float distSq = dist.lengthSquared();
        return {color * (color.a / distSq), dist / std::sqrt(distSq)};
    }
};

class DirectionalLight : public Light {
  public:
    DirectionalLight(Object *obj, Color color)
        : Light(obj, color) {}

    std::string name() { return "Directional Light"; }

    std::pair<Color, Vector3f> sample(Vector3f pos) {
        return {color * color.a, direction};
    }
    void update() {
        direction = Vector3f{0, 0, 1} * obj->transformRotation;
    }
  private:
    Vector3f direction;
};

class SpotLight : public Light {
  public:
    float spreadInner, spreadOuter;
    float spreadInnerCos, spreadOuterCos;
    Camera *shadowMap = nullptr;

    SpotLight(Object *obj, Color color, float spreadInner, float spreadOuter) 
    : Light(obj, color), spreadInner(spreadInner), spreadOuter(spreadOuter) {}

    std::string name() { return "Spotlight"; }

    std::pair<Color, Vector3f> sample(Vector3f pos);

    void update() {
        spreadInnerCos = std::cos(spreadInner);
        spreadOuterCos = std::cos(spreadOuter);
        if(spreadInnerCos < spreadOuterCos)
            std::swap(spreadInnerCos, spreadOuterCos);
        direction = Vector3f{0, 0, 1} * obj->transformRotation;
        
        if(shadowMap) {
            shadowMap->fov = std::max(spreadOuter, spreadInner) * (360.0f / M_PIf);
            shadowMap->render();
        }
    }

    void setupShadowMap(Vector2u size);
    void GUI();

  private:
    Vector3f direction;
};

#endif /* __LIGHT_H__ */
