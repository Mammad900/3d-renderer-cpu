#include "lua-state.h"
#include "../object.h"
#include <memory>
#include <algorithm>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaObject() {
        Lua.new_usertype<Component>("Component",
        sol::no_constructor,
        "object", sol::readonly(&Component::obj)
    );

    using ObjectVector = vector<shared_ptr<Object>>;
    Lua.new_usertype<ObjectVector>(
        "ObjectList", sol::constructors<ObjectVector()>(),
        "size", &ObjectVector::size,
        "at", [](ObjectVector& vec, size_t i) -> shared_ptr<Object> {
            return vec.at(i);
        }
    );

    using ComponentVector = vector<shared_ptr<Component>>;
    Lua.new_usertype<ComponentVector>(
        "ComponentList", sol::constructors<ComponentVector()>(),
        "size", &ComponentVector::size,
        "at", [](ComponentVector& vec, size_t i) -> shared_ptr<Component> {
            return vec.at(i);
        }
    );

    Lua.new_usertype<Object>("Object",
        "children", &Object::children,
        "components", &Object::components,
        "name", &Object::name,
        "position", &Object::position,
        "rotation", &Object::rotation,
        "scale", &Object::scale,
        sol::meta_function::construct, sol::overload(
            []() { return std::make_shared<Object>(); },
            [](sol::table properties) {
                shared_ptr<Object> obj = std::make_shared<Object>();
                obj->name = properties.get_or<std::string>("name", "");
                obj->position = valueFromObject<Vec3>(properties["position"]);
                obj->rotation = valueFromObject<Vec3>(properties["rotation"]);
                obj->scale = valueFromObject<Vec3>(properties["scale"], {1,1,1});

                if (sol::object children = properties["children"]; children.valid() && children.get_type() == sol::type::table) {
                    for (auto& kv : children.as<sol::table>()) {
                        auto child = kv.second.as<std::shared_ptr<Object>>();
                        obj->children.push_back(child);
                        if(!obj->scene.expired())
                            child->setScene(obj->scene);
                        child->parent = obj.get();
                    }
                }

                if (sol::object components = properties["components"]; components.valid() && components.get_type() == sol::type::table) {
                    for (auto& kv : components.as<sol::table>()) {
                        auto comp = kv.second.as<std::shared_ptr<Component>>();
                        comp->init(obj.get());
                        obj->components.push_back(comp);
                    }
                }

                return obj;
            }
        ),
        "add_child", [](Object& obj, shared_ptr<Object> child) {
            if(child->parent) { // it already has a parent, gotta remove it
                vector<shared_ptr<Object>> &vec = child->parent->children;
                vec.erase(std::remove(vec.begin(), vec.end(), child), vec.end());
            }
            obj.children.push_back(child);
            if(!obj.scene.expired() && (child->scene.expired() || obj.scene.lock() == child->scene.lock()))
                child->setScene(obj.scene);
            child->parent = &obj;
            child->update();
        },
        "add_component", [](Object& obj, shared_ptr<Component> component) {
            component->init(&obj);
            obj.components.push_back(std::move(component));
        },
        "transform", [](Object &obj, Vec3 vec) {
            return vec * obj.transform;
        },
        "transform_normal", [](Object &obj, Vec3 vec) {
            return vec * obj.transformNormals;
        },
        "transform_rotation", [](Object &obj, Vec3 vec) {
            return vec * obj.transformRotation;
        }
    );

    Lua.new_usertype<RotatorComponent>("RotatorComponent",
        sol::base_classes, sol::bases<Component>(),
        sol::meta_function::construct, [](sol::object rotatePerSecond) {
            return std::make_shared<RotatorComponent>(valueFromObject<Vec3>(rotatePerSecond));
        },
        "as_component", [](std::shared_ptr<RotatorComponent> &c) -> std::shared_ptr<Component> { return c; },
        "rotate_per_second", &RotatorComponent::rotatePerSecond,
        "enabled", &RotatorComponent::enable
    );
}