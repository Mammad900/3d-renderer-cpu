#include "lua-state.h"
#include "../light.h"

void luaLights() {
        Lua.new_usertype<Light>("Light",
        sol::no_constructor,
        "color", &Light::color,
        "as_component", [](std::shared_ptr<Light>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<PointLight>("PointLight",
        sol::meta_function::construct, [](Color color) {
            return std::make_shared<PointLight>( color);
        },
        "color", &PointLight::color,
        "as_component", [](std::shared_ptr<PointLight>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<DirectionalLight>("DirectionalLight",
        sol::meta_function::construct, [](Color color) {
            return std::make_shared<DirectionalLight>(color);
        },
        "color", &DirectionalLight::color,
        "as_component", [](std::shared_ptr<DirectionalLight>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<SpotLight>("SpotLight",
        sol::meta_function::construct, [](Color color, float spread_inner, float spread_outer) {
            return std::make_shared<SpotLight>(color, spread_inner, spread_outer);
        },
        "color", &SpotLight::color,
        "spread_inner", &SpotLight::spreadInner,
        "spread_outer", &SpotLight::spreadOuter,
        "as_component", [](std::shared_ptr<SpotLight>& l) -> std::shared_ptr<Component> { return l; }
    );
}