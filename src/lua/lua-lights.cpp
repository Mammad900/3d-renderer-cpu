#include "lua-state.h"
#include "../light.h"
#include "sol/sol.hpp"
#include <SFML/System/Vector2.hpp>
#include <memory>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaLights() {
        Lua.new_usertype<Light>("Light",
        sol::no_constructor,
        "color", &Light::color,
        "as_component", [](std::shared_ptr<Light>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<PointLight>("PointLight",
        sol::meta_function::construct, sol::overload(
            [](Color color) {
                return std::make_shared<PointLight>( color);
            },
            [](sol::table props) {
                shared_ptr<PointLight> light = std::make_shared<PointLight>(
                    valueFromObject<Color>(props["color"])
                );
                if(props["shadow_map"].valid()) {
                    sol::table shadowProps = props["shadow_map"];
                    light->setupShadowMap(valueFromObject<Vector2u>(shadowProps["size"]));
                    light->shadowMapCam->nearClip = shadowProps.get_or("near", 0.1f);
                    light->shadowMapCam->farClip = shadowProps.get_or("far", 100.f);
                }
                return light;
            }
        ),
        "color", &PointLight::color,
        "as_component", [](std::shared_ptr<PointLight>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<DirectionalLight>("DirectionalLight",
        sol::meta_function::construct, sol::overload(
            [](Color color) {
                return std::make_shared<DirectionalLight>(color);
            },
            [](sol::table props) {
                shared_ptr<DirectionalLight> light = std::make_shared<DirectionalLight>(
                    valueFromObject<Color>(props["color"])
                );
                if(props["shadow_map"].valid()) {
                    sol::table shadowProps = props["shadow_map"];
                    float fov = shadowProps.get_or("fov", 5.f);
                    light->setupShadowMap(valueFromObject<Vector2u>(shadowProps["size"]), fov);
                    light->litOutsideShadowMap = shadowProps.get_or("lit_outside", true);
                    light->shadowMapCam->nearClip = shadowProps.get_or("near", 0.1f);
                    light->shadowMapCam->farClip = shadowProps.get_or("far", 100.f);
                }
                return light;
            }
        ),
        "color", &DirectionalLight::color,
        "as_component", [](std::shared_ptr<DirectionalLight>& l) -> std::shared_ptr<Component> { return l; }
    );

    Lua.new_usertype<SpotLight>("SpotLight",
        sol::meta_function::construct, sol::overload(
            [](Color color, float spread_inner, float spread_outer) {
                return std::make_shared<SpotLight>(color, spread_inner, spread_outer);
            },
            [](sol::table props) {
                shared_ptr<SpotLight> light = std::make_shared<SpotLight>(
                    valueFromObject<Color>(props["color"]),
                    props.get_or("spread_inner", 0.2),
                    props.get_or("spread_outer", 0.3)
                );
                if(props["shadow_map"].valid()) {
                    sol::table shadowProps = props["shadow_map"];
                    light->setupShadowMap(valueFromObject<Vector2u>(shadowProps["size"]));
                    light->shadowMapCam->nearClip = shadowProps.get_or("near", 0.1f);
                    light->shadowMapCam->farClip = shadowProps.get_or("far", 100.f);
                }
                return light;
            }
        ),
        "color", &SpotLight::color,
        "spread_inner", &SpotLight::spreadInner,
        "spread_outer", &SpotLight::spreadOuter,
        "as_component", [](std::shared_ptr<SpotLight>& l) -> std::shared_ptr<Component> { return l; }
    );
}