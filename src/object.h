#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "matrix.h"
#include "miscTypes.h"

struct Object;

class Component {
  public:
    Object *obj;
    Component(Object *obj) : obj(obj) {}
    virtual void update(){};
    virtual void GUI(){};
};

struct Scene;

struct Object {
    Vector3f position;
    Vector3f rotation;
    Vector3f scale = {1,1,1};
    TransformMatrix myTransform;
    TransformMatrix myRotation;
    std::vector<Component *> components;
    Scene *scene;
    void update() {
        myRotation = makeRotationMatrix(rotation);
        myTransform = makeTransformMatrix(myRotation, scale, position);

        for (auto &&c : components)
            c->update();
    }
};

class MeshComponent : public Component {
  public:
    Mesh *mesh;
    MeshComponent(Object *obj, Mesh *mesh) : Component(obj), mesh(mesh) {}
};

#endif /* __OBJECT_H__ */
