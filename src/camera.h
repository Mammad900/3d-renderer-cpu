#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "object.h"
#include "data.h"

struct Projection {
    Vector3f worldPos;
    Vector3f screenPos;
    Vector3f normal;
};

class RenderTarget;

class Camera : public Component {
  public:
    Camera(Object *obj) : Component(obj) {}
    float fov = 60, nearClip = 0.1, farClip = 100;
    float whitePoint = 0;
    RenderTarget *tFrame;
    void render();
    std::string name() { return "Camera"; }
    void GUI();
    void fogPixel(int x, int y);

  private:
    Projection perspectiveProject(Vector3f a);
    void makePerspectiveProjectionMatrix();
    TransformMatrix projectionMatrix;
    float tanHalfFov;
};

void shutdownThreads();

#endif /* __CAMERA_H__ */
