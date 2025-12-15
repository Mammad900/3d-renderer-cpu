#include "lua-state.h"
#include "../color.h"
#include "../vector3.h"
#include "../miscTypes.h"
#include "../gui.h"

void luaSimples() {
        Lua.new_usertype<Color>("Color",
        "r", &Color::r,
        "g", &Color::g,
        "b", &Color::b,
        "a", &Color::a,
        sol::meta_function::construct, sol::overload(
            []() { return Color(); },
            [](float r, float g, float b, float a) { return Color(r, g, b, a); },
            [](sol::table t) {
                Color c{};
                c.r = t.get_or("r", 0.0f);
                c.g = t.get_or("g", 0.0f);
                c.b = t.get_or("b", 0.0f);
                c.a = t.get_or("a", 1.0f);
                return c;
            }
        )
    );
    
    Lua.new_usertype<Vec3>("Vec3",
        "x", &Vec3::x,
        "y", &Vec3::y,
        "z", &Vec3::z,
        sol::meta_function::construct, sol::overload(
            []() { return Vec3(); },
            [](float x, float y, float z) { return Vec3(x, y, z); },
            [](sol::table t) {
                Vec3 v{};
                v.x = t.get_or("x", 0.0f);
                v.y = t.get_or("y", 0.0f);
                v.z = t.get_or("z", 0.0f);
                return v;
            }
        )
    );
    Lua.set_function("deg_to_rad", [](sol::object deg) {
        return valueFromObject<Vec3>(deg) * (M_PI/180.0f);
    });
    Lua.set_function("rad_to_deg", [](sol::object rad) {
        return valueFromObject<Vec3>(rad) * (180.0f/M_PI);
    });

    Lua.new_usertype<Volume>(
        "Volume", sol::constructors<Volume()>(),
        "diffuse", &Volume::diffuse,
        "emissive", &Volume::emissive,
        "god_rays", &Volume::godRays,
        "god_rays_sample_size", &Volume::godRaysSampleSize,
        "transmission", sol::property(
            [](Volume &v) {
                return v.transmission;
            },
            [](Volume &v, Color value) {
                v.transmission = value;
                v.updateIntensity();
            }
        ),
        sol::meta_function::construct, sol::overload(
            []() { return std::make_shared<Volume>(); },
            [](sol::table t) {
                shared_ptr<Volume> v = std::make_shared<Volume>();
                v->name = t.get_or<std::string>("name", "");
                v->diffuse = valueFromObject(t["diffuse"], Color());
                v->emissive = valueFromObject(t["emissive"], Color());
                v->transmission = valueFromObject(t["transmission"], Color(1,1,1,0));
                v->godRays = t.get_or("god_rays", false);
                v->godRaysSampleSize = t.get_or("god_rays_sample_size", 1.0f);
                v->updateIntensity();
                volumes.emplace_back(v);
                return v;
            }
        )
    );
}