#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "matrix.h"
#include "miscTypes.h"

struct Object;
extern float deltaTime;

class Component {
  public:
    Object *obj;
    Component(Object *obj) : obj(obj) {}
    // Called after object transform updates
    virtual void update(){};
    // Called before object transform updates
    virtual void preUpdate(){};
    virtual void GUI(){};
    virtual std::string name() = 0;
};

struct Scene;

struct Object {
    std::string name;
    Vector3f position;
    Vector3f rotation;
    Vector3f scale = {1,1,1};

    std::vector<Component *> components;
    std::vector<Object *> children;
    Object *parent;
    Scene *scene;

    TransformMatrix transform;
    TransformMatrix transformRotation;
    TransformMatrix myTransform;
    TransformMatrix myTransformRotation;
    Vector3f globalPosition;

    void update();
    void GUI();
};

class MeshComponent : public Component {
  public:
    Mesh *mesh;
    MeshComponent(Object *obj, Mesh *mesh) : Component(obj), mesh(mesh) {}
    std::string name() { return "Mesh: " + mesh->label; }
};

class RotatorComponent : public Component {
  public:
    Vector3f rotatePerSecond;
    bool enable = true;
    RotatorComponent(Object *obj, Vector3f rotatePerSecond)
        : Component(obj), rotatePerSecond(rotatePerSecond) {}

    void preUpdate() { if(enable) obj->rotation += rotatePerSecond * deltaTime; }
    void GUI();
    std::string name() { return "Rotator"; }
};

#endif /* __OBJECT_H__ */
