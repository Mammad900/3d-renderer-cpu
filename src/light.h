#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "color.h"
#include "object.h"
#include "camera.h"

class Light : public Component {
  public:
    Color color;

    Light(Color color) : color(color) {}
    virtual ~Light();
    virtual std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene) = 0;
    virtual void update();

    void GUI();
  private:
    bool addedToScene = false;
};

class PointLight : public Light {
  public:
    PointLight(Color color)
        : Light(color) {}

    std::string name() { return "Point Light"; }

    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene) {
        Vec3 dist = pos - obj->globalPosition;
        float distSq = dist.lengthSquared();
        return {color * (color.a / distSq), dist / std::sqrt(distSq)};
    }
};

class DirectionalLight : public Light {
  public:
    DirectionalLight(Color color)
        : Light(color) {}

    std::string name() { return "Directional Light"; }

    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene) {
        return {color * color.a, direction};
    }
    void update() {
        Light::update();
        direction = Vec3{0, 0, 1} * obj->transformRotation;
    }
  private:
    Vec3 direction;
};

class SpotLight : public Light {
  public:
    float spreadInner, spreadOuter;
    float spreadInnerCos, spreadOuterCos;
    Camera *shadowMap = nullptr;

    SpotLight(Color color, float spreadInner, float spreadOuter) 
    : Light(color), spreadInner(spreadInner), spreadOuter(spreadOuter) {}

    std::string name() { return "Spotlight"; }

    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene);

    void update() {
        Light::update();
        spreadInnerCos = std::cos(spreadInner);
        spreadOuterCos = std::cos(spreadOuter);
        if(spreadInnerCos < spreadOuterCos)
            std::swap(spreadInnerCos, spreadOuterCos);
        direction = Vec3{0, 0, 1} * obj->transformRotation;
        
        if(shadowMap) {
            shadowMap->fov = std::max(spreadOuter, spreadInner) * (360.0f / M_PIf);
            shadowMap->render();
        }
    }

    void setupShadowMap(Vector2u size);
    void GUI();

  private:
    Vec3 direction;
};

#endif /* __LIGHT_H__ */
