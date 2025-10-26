#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "object.h"
#include <memory>

struct RenderTarget;

class Camera : public Component, public std::enable_shared_from_this<Camera> {
  public:
    float fov = 60, nearClip = 0.1, farClip = 100;
    float whitePoint = 0;
    bool shadowMap = false;
    bool shouldSetAsSceneCamera = false;
    RenderTarget *tFrame;
    void render();
    std::string name() { return "Camera"; }
    void GUI();
    void update();
    Projection perspectiveProject(Vec3 a);
    sf::Image getRenderedFrame(int renderMode);
    Vec3 screenSpaceToCameraSpace(int x, int y);
    Vec3 screenSpaceToCameraSpace(int x, int y, float z);
    Vec3 screenSpaceToWorldSpace(int x, int y);
    Vec3 screenSpaceToWorldSpace(int x, int y, float z);

  private:
    void makePerspectiveProjectionMatrix();
    void drawSkyBox();
    void buildTriangles(std::vector<TransparentTriangle> &transparents, std::vector<Triangle> &triangles);
    TransformMatrix projectionMatrix;
    float tanHalfFov;
};

void deferredPass(uint n, uint i0, Camera *camera);
void fogPass(uint n, uint i0, Camera *camera);

#endif /* __CAMERA_H__ */
