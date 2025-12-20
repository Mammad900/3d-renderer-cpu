#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "matrix.h"
#include "miscTypes.h"
#include <memory>

struct Object;

class Component {
  public:
    Object *obj = nullptr;
    // Called after object transform updates
    virtual void update(){};
    // Called before object transform updates
    virtual void preUpdate(){};
    virtual void init(Object *obj) {this->obj = obj;}
    virtual void GUI(){};
    virtual std::string name() = 0;
};

struct Scene;

struct Object {
    std::string name;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale = {1,1,1};

    std::vector<std::shared_ptr<Component>> components;
    std::vector<std::shared_ptr<Object>> children;
    Object *parent;
    std::weak_ptr<Scene> scene;

    TransformMatrix transform = identityMatrix;
    TransformMatrix transformRotation = identityMatrix;
    TransformMatrix myTransform = identityMatrix;
    TransformMatrix myTransformRotation = identityMatrix;
    TransformMatrix transformNormals = identityMatrix;
    Vec3 globalPosition;

    void update();
    void GUI();
    void setScene(std::weak_ptr<Scene> scene) {
        this->scene = scene;
        for (auto child : children)
            child->setScene(scene);
    }
};

class MeshComponent : public Component {
  public:
    shared_ptr<Mesh> mesh;
    MeshComponent(shared_ptr<Mesh> mesh) : mesh(mesh) {}
    std::string name() { return "Mesh: " + mesh->label; }
};

class RotatorComponent : public Component {
  public:
    Vec3 rotatePerSecond;
    bool enable = true;
    RotatorComponent(Vec3 rotatePerSecond) : rotatePerSecond(rotatePerSecond) {}

    void preUpdate();
    void GUI();
    std::string name() { return "Rotator"; }
};

#endif /* __OBJECT_H__ */
