#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "color.h"
#include "data.h"
#include "object.h"
#include "camera.h"

class Light : public Component {
  public:
    Color color;

    Light(Color color) : color(color) {}
    virtual ~Light();
    virtual std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene) = 0;
    virtual void update();
    virtual void updateShadowMap() {};
    float shadowSubsample(RenderTarget *frame, Vector2u pos, float dist);
    float shadowSample(RenderTarget *frame, Vec3 projected);

    void GUI();
  private:
    bool addedToScene = false;
};

class PointLight : public Light {
  public:
    shared_ptr<Camera> shadowMapCam = nullptr;
    std::array<shared_ptr<RenderTarget>, 6> shadowMaps;
    PointLight(Color color)
        : Light(color) {}

    std::string name() { return "Point Light"; }

    void updateShadowMap();
    void init(Object *obj);

    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene);
    void setupShadowMap(Vector2u size);
    void GUI();
};

class DirectionalLight : public Light {
  public:
    shared_ptr<Camera> shadowMapCam = nullptr;
    shared_ptr<RenderTarget> shadowMap = nullptr;
    bool litOutsideShadowMap = true;
    DirectionalLight(Color color)
        : Light(color) {}

    std::string name() { return "Directional Light"; }

    void updateShadowMap();
    void init(Object *obj);
    
    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene);
    void update() {
        Light::update();
        direction = Vec3{0, 0, 1} * obj->transformRotation;
    }
    void setupShadowMap(Vector2u size, float fov);
    void GUI();
  private:
    Vec3 direction;
};

class SpotLight : public Light {
  public:
    float spreadInner, spreadOuter;
    float spreadInnerCos, spreadOuterCos;
    shared_ptr<Camera> shadowMapCam = nullptr;
    shared_ptr<RenderTarget> shadowMap = nullptr;

    SpotLight(Color color, float spreadInner, float spreadOuter) 
    : Light(color), spreadInner(spreadInner), spreadOuter(spreadOuter) {}

    std::string name() { return "Spotlight"; }

    std::pair<Color, Vec3> sample(Vec3 pos, Scene &scene);

    void update();
    void updateShadowMap();
    void init(Object *obj);

    void setupShadowMap(Vector2u size);
    void GUI();

  private:
    Vec3 direction;
};

#endif /* __LIGHT_H__ */
