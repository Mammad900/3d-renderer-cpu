#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "object.h"

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

    std::pair<Color, Vector3f> sample(Vector3f pos) {
        Vector3f dist = pos - obj->position;
        float distSq = dist.lengthSquared();
        return {color * (color.a / distSq), dist / std::sqrt(distSq)};
    }
};

class DirectionalLight : public Light {
  public:
    DirectionalLight(Object *obj, Color color)
        : Light(obj, color) {}

    std::pair<Color, Vector3f> sample(Vector3f pos) {
        return {color * color.a, direction};
    }
    void update() {
        direction = Vector3f{0, 0, 1} * obj->myRotation;
    }
  private:
    Vector3f direction;
};

class SpotLight : public Light {
  public:
    float spreadInner, spreadOuter;
    float spreadInnerCos, spreadOuterCos;

    SpotLight(Object *obj, Color color, float spreadInner, float spreadOuter) 
    : Light(obj, color), spreadInner(spreadInner), spreadOuter(spreadOuter) {}

    std::pair<Color, Vector3f> sample(Vector3f pos);

    void update() {
        spreadInnerCos = std::cos(spreadInner);
        spreadOuterCos = std::cos(spreadOuter);
        if(spreadInnerCos < spreadOuterCos)
            std::swap(spreadInnerCos, spreadOuterCos);
        direction = Vector3f{0, 0, 1} * obj->myRotation;
    }
    void GUI();
  private:
    Vector3f direction;
};

#endif /* __LIGHT_H__ */
