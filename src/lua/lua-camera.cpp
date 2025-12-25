#include "lua-state.h"
#include "../camera.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaCamera() {
    Lua.new_usertype<Camera>("Camera",
        sol::base_classes, sol::bases<Component>(),
        sol::meta_function::construct, sol::overload(
            [](){ return std::make_shared<Camera>(); },
            [](sol::table properties) {
                shared_ptr<Camera> camera = std::make_shared<Camera>();
                camera->fov = properties.get_or("fov", camera->fov);
                camera->nearClip = properties.get_or("near", camera->nearClip);
                camera->farClip = properties.get_or("far", camera->farClip);
                camera->whitePoint = properties.get_or("white_point", camera->whitePoint);
                camera->orthographic = properties.get_or("orthographic", camera->orthographic);
                return camera;
            }
        ),
        "fov", &Camera::fov,
        "near", &Camera::nearClip,
        "far", &Camera::farClip,
        "white_point", &Camera::whitePoint,
        "as_component", [](shared_ptr<Camera> &c)-> shared_ptr<Component> { return c; }
    );
}