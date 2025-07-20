#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "matrix.h"
#include "miscTypes.h"

struct Object;

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
    Vec3 position;
    Vec3 rotation;
    Vec3 scale = {1,1,1};

    std::vector<Component *> components;
    std::vector<Object *> children;
    Object *parent;
    Scene *scene;

    TransformMatrix transform;
    TransformMatrix transformRotation;
    TransformMatrix myTransform;
    TransformMatrix myTransformRotation;
    TransformMatrix transformNormals;
    Vec3 globalPosition;

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
    Vec3 rotatePerSecond;
    bool enable = true;
    RotatorComponent(Object *obj, Vec3 rotatePerSecond)
        : Component(obj), rotatePerSecond(rotatePerSecond) {}

    void preUpdate();
    void GUI();
    std::string name() { return "Rotator"; }
};

class KeyboardControlComponent : public Component {
    // This doesn't actually do anything itself. 
    // sceneFile assigns scene->keyboardControl to this
    // And main.cpp does the input handling
  public:
    Vec3 speed = {1,1,1};
    KeyboardControlComponent(Object *obj, Vec3 speed)
        : Component(obj), speed(speed) {}
    std::string name() { return "Keyboard Control"; }
    void GUI();
};

#endif /* __OBJECT_H__ */
