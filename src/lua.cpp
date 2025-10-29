#include "lua.h"
#include "camera.h"
#include "color.h"
#include "data.h"
#include "generateMesh.h"
#include "gui.h"
#include "miscTypes.h"
#include "object.h"
#include "pbrMaterial.h"
#include "phongMaterial.h"
#include "earthMaterial.h"
#include "texture.h"
#include "textureFiltering.h"
#include "tinyTexture.h"
#include "vector3.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <filesystem>
#include <memory>
#include <string>
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "material.h"

sol::state Lua;
std::vector<std::function<void(float)>> onFrameCallbacks;

void luaOnFrame() {
    for (auto& cb : onFrameCallbacks)
        cb(timing.deltaTime);
}

void luaDestroy() {
    onFrameCallbacks.clear(); // otherwise the lua functions outliving the lua state will cause a segfault.
}

template<typename T>
T valueFromObject(sol::object obj, T def = T{}) {
    if constexpr (std::is_same_v<T, float>) {
        // Float: only accept a number
        if (obj.get_type() == sol::type::number)
            return obj.as<float>();
        return def;
    }
    else if constexpr (std::is_same_v<T, Color>) {
        Color v = def;
        if (obj.get_type() == sol::type::userdata)
            v = obj.as<Color>();
        else if (obj.get_type() == sol::type::number) {
            float c = obj.as<float>();
            v = Color{c, c, c, 1.0f};
        } else if (obj.get_type() == sol::type::table) {
            sol::table t = obj;
            v = Color{
                t.get_or(1, 1.0f),
                t.get_or(2, 1.0f),
                t.get_or(3, 1.0f),
                t.get_or(4, 1.0f)
            };
        }
        return v;
    }
    else if constexpr (std::is_same_v<T, Vec3>) {
        Vec3 v = def;
        if (obj.get_type() == sol::type::userdata)
            v = obj.as<Vec3>();
        else if (obj.get_type() == sol::type::number) {
            float val = obj.as<float>();
            v = Vec3{val, val, val};
        } else if (obj.get_type() == sol::type::table) {
            sol::table t = obj;
            v = Vec3{
                t.get_or(1, 0.0f),
                t.get_or(2, 0.0f),
                t.get_or(3, 0.0f)
            };
        }
        return v;
    }
    else {
        static_assert(!sizeof(T*), "Unsupported type in valueFromObject");
    }
}

std::filesystem::path get_calling_script_path(sol::state_view& lua) {
    sol::table debug = lua["debug"];
    if (!debug.valid()) return "./";
    
    sol::protected_function getinfo = debug["getinfo"];
    if (!getinfo.valid()) return "./";

    sol::protected_function_result result = getinfo(2, "S");
    if (!result.valid()) return "./";

    sol::table info = result;
    std::string source = info["source"].get_or<std::string>("");
    if (source.size() > 0 && source[0] == '@')
        source = source.substr(1);

    return std::filesystem::path(source).parent_path();
};

template <typename T>
void makeTextureUsertypes(std::string name) {
    Lua.new_usertype<Texture<T>>(name+"Texture",
        sol::no_constructor
    );

    Lua.new_usertype<SolidTexture<T>>("Solid"+name+"Texture",
        sol::meta_function::construct, [](sol::object value) {
            return std::make_shared<SolidTexture<T>>(valueFromObject<T>(value));
        },
        "value", &SolidTexture<T>::value,
        "as_texture", [](std::shared_ptr<SolidTexture<T>>& l) -> std::shared_ptr<Texture<T>> { return l; }
    );

    Lua.new_usertype<ImageTexture<T>>("Image"+name+"Texture",
        sol::meta_function::construct, [](sol::this_state s, std::string path, sol::object scale_in)-> shared_ptr<ImageTexture<T>> {
            sol::state_view lua(s);
            sf::Image image;
            std::cout << "Loading texture " << path << std::flush;
            if (!image.loadFromFile(get_calling_script_path(lua) / path)) {
                std::cerr << "\nFailed to load image: " << path << std::endl;
                return std::make_shared<ErrorTexture<T, ImageTexture<T>>>();
            }
            std::cout << "." << std::flush;

            T def;
            if constexpr (std::is_same_v<T, float>)
                def = 1.0f;
            if constexpr (std::is_same_v<T, Color>)
                def = Color{1,1,1,1};
            if constexpr (std::is_same_v<T, Vec3>)
                def = Vec3{1,1,1};
            auto tex = std::make_shared<ImageTexture<T>>(image, valueFromObject<T>(scale_in, def));
            std::cout << "." << std::endl;
            return tex;
        },
        "size", sol::readonly(&ImageTexture<T>::size),
        "scale", &ImageTexture<T>::value,
        "as_texture", [](std::shared_ptr<ImageTexture<T>>& l) -> std::shared_ptr<Texture<T>> { return l; }
    );
}

template<typename T, typename P>
void makeBlendTextureUsertype(std::string nameT, std::string nameP) {
    using BlendTex = BlendTexture<T, P>;

    Lua.new_usertype<BlendTex>(
        "Blend" + nameT + nameP + "Texture",
        sol::meta_function::construct, [](std::shared_ptr<Texture<T>> a, std::string mode_str, std::shared_ptr<Texture<P>> b) {
            BlendMode mode;
            if (mode_str == "alpha_mix") mode = BlendMode::AlphaMix;
            else if (mode_str == "add") mode = BlendMode::Add;
            else if (mode_str == "multiply") mode = BlendMode::Multiply;
            else if (mode_str == "subtract") mode = BlendMode::Subtract;
            else throw std::runtime_error("Invalid blend mode: " + mode_str);
            return std::make_shared<BlendTex>(a, b, mode);
        },
        "a", &BlendTex::a,
        "b", &BlendTex::b,
        "mode", sol::property(
            [](const BlendTex &self) {
                switch (self.mode) {
                    case BlendMode::AlphaMix: return std::string("alpha_mix");
                    case BlendMode::Add: return std::string("add");
                    case BlendMode::Multiply: return std::string("multiply");
                    case BlendMode::Subtract: return std::string("subtract");
                    default: return std::string("unknown");
                }
            },
            [](BlendTex &self, const std::string &mode_str) {
                if (mode_str == "alpha_mix") self.mode = BlendMode::AlphaMix;
                else if (mode_str == "add") self.mode = BlendMode::Add;
                else if (mode_str == "multiply") self.mode = BlendMode::Multiply;
                else if (mode_str == "subtract") self.mode = BlendMode::Subtract;
                else throw std::runtime_error("Invalid blend mode: " + mode_str);
            }
        ),
        "as_texture", [](std::shared_ptr<BlendTex>& l) -> std::shared_ptr<Texture<T>> { return l; }
    );
}


void lua(std::string path) {
	Lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::debug, sol::lib::package);

#pragma region Simple types
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
        "transmission", &Volume::transmission,
        sol::meta_function::construct, sol::overload(
            []() { return std::make_shared<Volume>(); },
            [](sol::table t) {
                shared_ptr<Volume> v = std::make_shared<Volume>();
                v->name = t.get_or<std::string>("name", "");
                v->diffuse = valueFromObject(t["diffuse"], Color());
                v->emissive = valueFromObject(t["emissive"], Color());
                v->transmission = valueFromObject(t["transmission"], Color(1,1,1,0));
                volumes.emplace_back(v);
                return v;
            }
        )
    );
#pragma endregion

#pragma region Scene Comopenent Object
    Lua.new_usertype<Scene>("Scene",
        sol::meta_function::construct, []() { return std::make_shared<Scene>(); },
        "name", &Scene::name,
        "sky_box", &Scene::skyBox,
        "set_active_camera", &Scene::setActiveCamera,
        "objects", &Scene::objects,
        "add_object", [](Scene& scene, shared_ptr<Object> child) {
            child->setScene(scene.shared_from_this());
            child->parent = nullptr;
            scene.objects.push_back(std::move(child));
        },
        "add_objects", [](Scene &scene, sol::table children) {
            for (auto& kv : children.as<sol::table>()) {
                auto child = kv.second.as<std::shared_ptr<Object>>();
                child->setScene(scene.shared_from_this());
                child->parent = nullptr;
                scene.objects.push_back(child);
            }
        },
        "back_face_culling", &Scene::backFaceCulling,
        "ambient_light", &Scene::ambientLight,
        "volume", &Scene::volume,
        "god_rays", &Scene::godRays,
        "god_rays_sample_size", &Scene::godRaysSampleSize,
        "bilinear_shadow_filtering", &Scene::bilinearShadowFiltering,
        "shadow_bias", &Scene::shadowBias,
        "texture_filtering_mode", sol::property(
            [](Scene &s) {
                using enum TextureFilteringMode;
                switch (s.textureFilteringMode) {
                case NearestNeighbor:
                    return "nearest_neighbor";
                case Bilinear:
                    return "bilinear";
                case Trilinear:
                    return "trilinear";
                case None:
                default:
                    return "none";
                }
            },
            [](Scene &s, std::string mode) {
                using enum TextureFilteringMode;
                if(mode == "nearest_neighbor")
                    s.textureFilteringMode = NearestNeighbor;
                else if(mode == "bilinear")
                    s.textureFilteringMode = Bilinear;
                else if(mode == "trilinear")
                    s.textureFilteringMode = Trilinear;
            }
        )
    );
    Lua["set_render_scene"] = [](shared_ptr<Scene> s) {
        scene = s;
    };

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
            obj.children.push_back(std::move(child));
            if(!obj.scene.expired())
                child->setScene(obj.scene);
            child->parent = &obj;
        },
        "add_component", [](Object& obj, shared_ptr<Component> component) {
            component->init(&obj);
            obj.components.push_back(std::move(component));
        }
    );
#pragma endregion

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
                camera->shouldSetAsSceneCamera = properties.get_or("active", false);
                return camera;
            }
        ),
        "fov", &Camera::fov,
        "near", &Camera::nearClip,
        "far", &Camera::farClip,
        "white_point", &Camera::whitePoint,
        "as_component", [](shared_ptr<Camera> &c)-> shared_ptr<Component> { return c; }
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

#pragma region Materials
    Lua.new_usertype<Material>("Material",
        sol::no_constructor,
        "transparent", sol::property(
            [](Material& m){ return m.flags.transparent; },
            [](Material& m, bool v){ m.flags.transparent = v; }
        ),
        "double_sided", sol::property(
            [](Material& m){ return m.flags.doubleSided; },
            [](Material& m, bool v){ m.flags.doubleSided = v; }
        ),
        "alpha_cutout", sol::property(
            [](Material& m){ return m.flags.alphaCutout; },
            [](Material& m, bool v){ m.flags.alphaCutout = v; }
        )
    );

    Lua.new_usertype<PhongMaterial>("PhongMaterial",
        sol::base_classes, sol::bases<Material>(),
        sol::meta_function::construct, [](sol::table properties) {
            PhongMaterialProps props;
            props.diffuse = properties.get_or("diffuse", props.diffuse);
            props.specular = properties.get_or("specular", props.specular);
            props.tint = properties.get_or("tint", props.tint);
            props.emissive = properties.get_or("emissive", props.emissive);
            props.normalMap = properties.get_or("normal_map", props.normalMap);

            auto mat = std::make_shared<PhongMaterial>(
                props,
                properties.get_or<std::string>("name", "Phong"),
                MaterialFlags{
                    .transparent = properties.get_or("transparent", false),
                    .doubleSided = properties.get_or("double_sided", false),
                    .alphaCutout = properties.get_or("alpha_cutout", false),
                },
                properties.get_or<shared_ptr<Volume>>("volume_front", nullptr),
                properties.get_or<shared_ptr<Volume>>("volume_back", nullptr)
            );
            materials.emplace_back(mat);
            return mat;
        },
        "as_material", [](shared_ptr<PhongMaterial> &c)-> shared_ptr<Material> { return c; }
    );

    Lua.new_usertype<EarthMaterial>("EarthMaterial",
        sol::base_classes, sol::bases<Material>(),
        sol::meta_function::construct, [](sol::table properties) {
            std::string name = properties.get_or<std::string>("name", "Earth");

            auto mat = std::make_shared<EarthMaterial>(name);
            materials.emplace_back(mat);

            // Terrain textures
            mat->terrainMat->mat.diffuse   = properties.get_or("terrain_diffuse", mat->terrainMat->mat.diffuse);
            mat->terrainMat->mat.emissive  = properties.get_or("city_lights", mat->terrainMat->mat.emissive);
            mat->terrainMat->mat.normalMap = properties.get_or("normal_map", mat->terrainMat->mat.normalMap);
            
            // Ocean textures
            mat->oceanMat->mat.diffuse     = properties.get_or("ocean_diffuse", mat->oceanMat->mat.diffuse);
            mat->oceanMat->mat.specular    = properties.get_or("ocean_specular", mat->oceanMat->mat.specular);
            mat->oceanMask                 = properties.get_or("ocean_mask", mat->oceanMask);

            // Cloud textures
            mat->cloudMat->mat.diffuse     = properties.get_or("cloud_diffuse", mat->cloudMat->mat.diffuse);
            mat->cloudTexture              = properties.get_or("cloud_texture", mat->cloudTexture);

            return mat;
        },
        "as_material", [](std::shared_ptr<EarthMaterial> &c) -> std::shared_ptr<Material> { return c; },

        // Expose texture handles for Lua access
        "terrain_diffuse", sol::property(
            [](EarthMaterial &self) { return self.terrainMat->mat.diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.terrainMat->mat.diffuse = tex; }
        ),
        "city_lights", sol::property(
            [](EarthMaterial &self) { return self.terrainMat->mat.emissive; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.terrainMat->mat.emissive = tex; }
        ),
        "ocean_diffuse", sol::property(
            [](EarthMaterial &self) { return self.oceanMat->mat.diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.oceanMat->mat.diffuse = tex; }
        ),
        "ocean_specular", sol::property(
            [](EarthMaterial &self) { return self.oceanMat->mat.specular; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.oceanMat->mat.specular = tex; }
        ),
        "cloud_diffuse", sol::property(
            [](EarthMaterial &self) { return self.cloudMat->mat.diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.cloudMat->mat.diffuse = tex; }
        ),
        "ocean_mask", sol::property(
            [](EarthMaterial &self) { return self.oceanMask; },
            [](EarthMaterial &self, std::shared_ptr<Texture<float>> tex) { self.oceanMask = tex; }
        ),
        "cloud_texture", sol::property(
            [](EarthMaterial &self) { return self.cloudTexture; },
            [](EarthMaterial &self, std::shared_ptr<Texture<float>> tex) { self.cloudTexture = tex; }
        )
    );

    Lua.new_usertype<PBRMaterial>("PBRMaterial",
        sol::base_classes, sol::bases<Material>(),

        sol::meta_function::construct, [](sol::table properties) {

            shared_ptr<Texture<Color>> albedo = properties.get_or<shared_ptr<Texture<Color>>>("albedo", nullptr);
            shared_ptr<Texture<float>> metallic = properties.get_or<shared_ptr<Texture<float>>>("metallic", nullptr);
            shared_ptr<Texture<float>> roughness = properties.get_or<shared_ptr<Texture<float>>>("roughness", nullptr);
            shared_ptr<Texture<float>> ambient_occlusion = properties.get_or<shared_ptr<Texture<float>>>("ambient_occlusion", nullptr);

            auto mat= std::make_shared<PBRMaterial>(
                properties.get_or("name", std::string("PBR")), 
                MaterialFlags{
                    .transparent = properties.get_or("transparent", false),
                    .doubleSided = properties.get_or("double_sided", false),
                    .alphaCutout = properties.get_or("alpha_cutout", false),
                },
                albedo ? albedo : std::make_shared<SolidTexture<Color>>(Color{1,1,1,1}),
                metallic ? metallic : std::make_shared<SolidTexture<float>>(0),
                roughness ? roughness : std::make_shared<SolidTexture<float>>(1),
                ambient_occlusion ? ambient_occlusion : std::make_shared<SolidTexture<float>>(1)
            );
            materials.emplace_back(mat);
            return mat;
        },

        "flags", &PBRMaterial::flags,
        "albedo", &PBRMaterial::albedo,
        "metallic", &PBRMaterial::metallic,
        "roughness", &PBRMaterial::roughness,
        "ambient_occlusion", &PBRMaterial::ambientOcclusion,
        "as_material", [](std::shared_ptr<PBRMaterial>& m) -> std::shared_ptr<Material> { return m; }
    );


#pragma endregion

#pragma region Mesh
    auto VertexConstructor = [](sol::table t) {
        Vertex v{};

        auto parse_vec3 = [&](const char* key, Vec3 def) -> Vec3 {
            sol::object o = t[key];
            if (!o.valid()) return def;

            // If it's already a Vec3 userdata
            if (o.is<Vec3>()) {
                return o.as<Vec3>();
            }

            // If it's a table, try numeric indices first, then x,y,z fields
            if (o.get_type() == sol::type::table) {
                sol::table tt = o;
                double x = 0, y = 0, z = 0;

                x = tt.get_or(1, static_cast<double>(def.x));
                y = tt.get_or(2, static_cast<double>(def.y));
                z = tt.get_or(3, static_cast<double>(def.z));
                return Vec3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
            }

            // If it's a number (unlikely) or otherwise, fallback
            return def;
        };

        auto parse_vec2 = [&](const char* key, Vector2f def) -> Vector2f {
            sol::object o = t[key];
            if (!o.valid()) return def;

            if (o.is<Vector2f>()) {
                return o.as<Vector2f>();
            }

            if (o.get_type() == sol::type::table) {
                sol::table tt = o;
                double x = 0, y = 0;

                x = tt.get_or(1, static_cast<double>(def.x));
                y = tt.get_or(2, static_cast<double>(def.y));
                return Vector2f{static_cast<float>(x), static_cast<float>(y)};
            }

            return def;
        };

        // defaults come from value-initialized Vertex (or supply your own)
        Vec3 default_pos = v.position;
        Vector2f default_uv = v.uv;
        Vec3 default_nrm = v.normal;

        v.position = parse_vec3("position", default_pos);
        v.uv       = parse_vec2("uv", default_uv);
        v.normal   = parse_vec3("normal", default_nrm);

        return v;
    };
    Lua.new_usertype<Vertex>("Vertex",
        sol::constructors<Vertex()>(),
        "position", &Vertex::position,
        "uv", &Vertex::uv,
        "normal", &Vertex::normal,
        sol::meta_function::construct, sol::overload(
            []() { return Vertex(); },
            VertexConstructor
        )
    );

    auto FaceConstructor = [](sol::table t) {
        Face f{};
        f.v1 = t.get_or("v1", static_cast<uint16_t>(t.get_or(1, 0)));
        f.v2 = t.get_or("v2", static_cast<uint16_t>(t.get_or(2, 0)));
        f.v3 = t.get_or("v3", static_cast<uint16_t>(t.get_or(3, 0)));
        f.material = t.get_or("material", nullptr);
        return f;
    };
    Lua.new_usertype<Face>("Face",
        sol::constructors<Face()>(),
        "v1", &Face::v1,
        "v2", &Face::v2,
        "v3", &Face::v3,
        "material", &Face::material,
        sol::meta_function::construct, sol::overload(
            [](uint16_t a, uint16_t b, uint16_t c, shared_ptr<Material> mat) {
                Face f{};
                f.v1 = a; f.v2 = b; f.v3 = c; f.material = mat;
                return f;
            },
            FaceConstructor
        )
    );

    Lua.new_usertype<Mesh>("Mesh",
        // no default value semantics from Lua; return shared_ptr so Lua can own it
        sol::meta_function::construct, sol::overload(
            []() { return std::make_shared<Mesh>(); },
            [VertexConstructor](sol::table t) -> std::shared_ptr<Mesh> {
                Mesh mesh;

                mesh.label = t.get_or<std::string>("label", "");
                mesh.flatShading = t.get_or("flat_shading", false);

                // vertices: iterate the Lua table and convert each entry to Vertex
                sol::object verts_obj = t["vertices"];
                if (verts_obj.valid() && verts_obj.get_type() == sol::type::table) {
                    sol::table verts = verts_obj;
                    for (auto& pair : verts) {
                        sol::object elem = pair.second;
                        // allow already-constructed Vertex userdata or plain tables
                        if (elem.get_type() == sol::type::userdata) {
                            mesh.vertices.push_back(elem.as<Vertex>());
                        } else if(elem.get_type() == sol::type::table) {
                            sol::table ftable = elem;
                            if (ftable[1].valid() || ftable[2].valid() || ftable[3].valid()) {
                            } else {
                                mesh.vertices.push_back(VertexConstructor(ftable));
                            }
                        }
                    }
                }

                // faces: accept either Face userdata/table or simple array-like {1,2,3}
                sol::object faces_obj = t["faces"];
                if (faces_obj.valid() && faces_obj.get_type() == sol::type::table) {
                    sol::table faces = faces_obj;
                    for (auto& pair : faces) {
                        sol::object elem = pair.second;
                        if (elem.get_type() == sol::type::userdata) {
                            mesh.faces.push_back(elem.as<Face>());
                        } else if (elem.get_type() == sol::type::table) {
                            sol::table ft = elem;
                            Face f;
                            f.v1 = static_cast<uint16_t>(ft.get_or(1, 0));
                            f.v2 = static_cast<uint16_t>(ft.get_or(2, 0));
                            f.v3 = static_cast<uint16_t>(ft.get_or(3, 0));
                            f.material = ft.get_or<shared_ptr<Material>>(4, nullptr);
                            mesh.faces.push_back(f);
                        }
                    }
                }

                if(t.get_or("auto_normals", false)) {
                    bakeMeshNormals(mesh);
                }

                auto meshPtr = std::make_shared<Mesh>(std::move(mesh));
                meshes.emplace_back(meshPtr);
                return meshPtr;
            }
        ),

        // expose fields for read/write from Lua
        "label", &Mesh::label,
        // "vertices", &Mesh::vertices, // vector<Vertex> (sol can handle vectors if Vertex usertype exists) // no it cant
        // "faces", &Mesh::faces,       // vector<Face>
        "flatShading", &Mesh::flatShading
    );

    Lua.new_usertype<MeshComponent>("MeshComponent",
        sol::base_classes, sol::bases<Component>(),
        sol::meta_function::construct, [](shared_ptr<Mesh> mesh) {
            return std::make_shared<MeshComponent>(mesh);
        },
        "as_component", [](shared_ptr<MeshComponent> &c)-> shared_ptr<Component> { return c; }
    );

    Lua["generate_mesh"] = [](sol::this_state s, sol::table t) {
        sol::state_view lua(s);
        std::string type = t["type"];
        shared_ptr<Mesh> mesh;
        if(type == "sphere") {
            mesh = createSphere(
                t["material"], 
                t.get_or<std::string>("name", "Sphere"), 
                t.get_or("stacks", 20), 
                t.get_or("sectors", 40), 
                t.get_or("invert_u", false),
                t.get_or("invert_v", false)
            );
        } else if(type == "plane") {
            mesh = createPlane(
                t["material"], 
                t.get_or<std::string>("name", "Plane"), 
                t.get_or("subdivisions_x", 1), 
                t.get_or("subdivisions_y", 1)
            );
        } else if (type == "obj") {
            mesh = loadOBJ(
                get_calling_script_path(lua) / t.get<std::string>("file"), 
                t["material"], 
                t.get_or<std::string>("name", t["file"])
            );
        } else if (type == "stl") {
            mesh = loadSTL(
                get_calling_script_path(lua) / t.get<std::string>("file"), 
                t["material"], 
                t.get_or<std::string>("name", t["file"])
            );
        } else if (type == "regular_icosahedron") {
            mesh = makeRegularIcosahedron(
                t.get_or<std::string>("name", "Regular Icosahedron"), 
                t["material"]
            );
        } else if (type == "ico_sphere") {
            mesh = makeIcoSphere(
                t.get_or<std::string>("name", "IcoSphere"), 
                t["material"],
                t.get_or("subdivisions", 3)
            );
        } else if (type == "dodecahedron") {
            bool pentakis = t.get_or("is_pentakis", false);
            mesh = makeDodecahedron(
                t.get_or<std::string>("name", pentakis ? "Pentakis Dodecahedron" : "Regular Dodecahedron"), 
                t["material"], 
                pentakis
            );
        } else if (type == "truncated_icosahedron") {
            mesh = makeTruncatedIcosahedron(
                t.get_or<std::string>("name", "Truncated Icosahedron"), 
                t["material"]
            );
        } else if (type == "ball") {
            shared_ptr<Material> mat = t.get_or("material", nullptr);
            mesh = makeBall(
                t.get_or<std::string>("name", "Ball"), 
                t.get_or("hexagons_material", mat), 
                t.get_or("pentagons_material", mat), 
                t.get_or("subdivisions", 2)
            );
        } else if (type == "cube_sphere") {
            std::array<std::shared_ptr<Material>, 6> mats;
            bool singleTexture = false;
            if (t["material"].valid()) {
                auto mat = t["material"].get<std::shared_ptr<Material>>();
                mats.fill(mat);
                singleTexture = true;
            } else if (t["materials"].valid()) {
                sol::table tbl = t["materials"];
                for (int i = 0; i < 6; i++) {
                    mats[i] = tbl[i + 1].get<std::shared_ptr<Material>>();
                }
            }
            mesh = makeCubeSphere(
                t.get_or<std::string>("name", "Cube Sphere"), 
                mats, 
                t.get_or("subdivisions", 10),
                singleTexture
            );
        }
        else mesh = std::make_shared<Mesh>();
        mesh->flatShading = t.get_or("flat_shading", mesh->flatShading);
        meshes.emplace_back(mesh);
        return mesh;
    };
#pragma endregion

#pragma region Lights
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
        "spread_outer", &SpotLight::spreadOuter
    );
#pragma endregion
    
#pragma region Textures

    makeTextureUsertypes<Color>("Color");
    makeTextureUsertypes<Vec3>("Vector");
    makeTextureUsertypes<float>("Float");

    Lua.new_usertype<SineWaveTexture>("SineWaveTexture",
        sol::meta_function::construct, [](float a, float b, float c, float d, float e, bool orientation) {
            return std::make_shared<SineWaveTexture>(a, b, c, d, e, orientation);
        },
        "a", &SineWaveTexture::a,
        "b", &SineWaveTexture::b,
        "c", &SineWaveTexture::c,
        "d", &SineWaveTexture::d,
        "e", &SineWaveTexture::e,
        "orientation", &SineWaveTexture::orientation,
        "as_texture", [](std::shared_ptr<SineWaveTexture>& t) -> std::shared_ptr<Texture<float>> { return t; }
    );

    makeBlendTextureUsertype<Color, Color>("Color", "Color");
    makeBlendTextureUsertype<Color, float>("Color", "Float");
    makeBlendTextureUsertype<Vec3, float>("Vector", "Float");
    makeBlendTextureUsertype<float, float>("Float", "Float");


    Lua.new_usertype<TinyImageTexture>("TinyImageTexture",
        sol::meta_function::construct, [](sol::this_state s, std::string path, sol::object scale_in)-> shared_ptr<TinyImageTexture> {
            sol::state_view lua(s);
            sf::Image image;
            std::cout << "Loading texture " << path << std::flush;
            if (!image.loadFromFile(get_calling_script_path(lua) / path)) {
                std::cerr << "\nFailed to load image: " << path << std::endl;
                return std::make_shared<ErrorTexture<Color, TinyImageTexture>>();
            }
            std::cout << "." << std::flush;
            shared_ptr<TinyImageTexture> tex = std::make_shared<TinyImageTexture>(image, valueFromObject<Color>(scale_in, Color{1,1,1,1}));
            std::cout << "." << std::endl;
            return tex;
        },
        "size", sol::readonly_property([](TinyImageTexture& l){return l.image.getSize();}),
        "scale", &TinyImageTexture::value,
        "as_texture", [](std::shared_ptr<TinyImageTexture>& l) -> std::shared_ptr<Texture<Color>> { return l; }
    );

#pragma endregion

#pragma region Scripting
    Lua.set_function("on_frame", [](sol::function f) {
        onFrameCallbacks.emplace_back([f](float dt) {
            if(f.valid())
                f(dt); // call Lua function every frame
        });
    });
    Lua.set_function("is_key_pressed", [](int key) {
        return sf::Keyboard::isKeyPressed((sf::Keyboard::Key)key);
    });
    using sf::Keyboard::Key;
    Lua["key"] = Lua.create_table_with(
        "unknown", Key::Unknown,
        "a", Key::A,
        "b", Key::B,
        "c", Key::C,
        "d", Key::D,
        "e", Key::E,
        "f", Key::F,
        "g", Key::G,
        "h", Key::H,
        "i", Key::I,
        "j", Key::J,
        "k", Key::K,
        "l", Key::L,
        "m", Key::M,
        "n", Key::N,
        "o", Key::O,
        "p", Key::P,
        "q", Key::Q,
        "r", Key::R,
        "s", Key::S,
        "t", Key::T,
        "u", Key::U,
        "v", Key::V,
        "w", Key::W,
        "x", Key::X,
        "y", Key::Y,
        "z", Key::Z,
        "num0", Key::Num0,
        "num1", Key::Num1,
        "num2", Key::Num2,
        "num3", Key::Num3,
        "num4", Key::Num4,
        "num5", Key::Num5,
        "num6", Key::Num6,
        "num7", Key::Num7,
        "num8", Key::Num8,
        "num9", Key::Num9,
        "escape", Key::Escape,
        "l_control", Key::LControl,
        "l_shift", Key::LShift,
        "l_alt", Key::LAlt,
        "l_system", Key::LSystem,
        "r_control", Key::RControl,
        "r_shift", Key::RShift,
        "r_alt", Key::RAlt,
        "r_system", Key::RSystem,
        "menu", Key::Menu,
        "l_bracket", Key::LBracket,
        "r_bracket", Key::RBracket,
        "semicolon", Key::Semicolon,
        "comma", Key::Comma,
        "period", Key::Period,
        "apostrophe", Key::Apostrophe,
        "slash", Key::Slash,
        "backslash", Key::Backslash,
        "grave", Key::Grave,
        "equal", Key::Equal,
        "hyphen", Key::Hyphen,
        "space", Key::Space,
        "enter", Key::Enter,
        "backspace", Key::Backspace,
        "tab", Key::Tab,
        "page_up", Key::PageUp,
        "page_down", Key::PageDown,
        "end", Key::End,
        "home", Key::Home,
        "insert", Key::Insert,
        "delete", Key::Delete,
        "add", Key::Add,
        "subtract", Key::Subtract,
        "multiply", Key::Multiply,
        "divide", Key::Divide,
        "left", Key::Left,
        "right", Key::Right,
        "up", Key::Up,
        "down", Key::Down,
        "numpad0", Key::Numpad0,
        "numpad1", Key::Numpad1,
        "numpad2", Key::Numpad2,
        "numpad3", Key::Numpad3,
        "numpad4", Key::Numpad4,
        "numpad5", Key::Numpad5,
        "numpad6", Key::Numpad6,
        "numpad7", Key::Numpad7,
        "numpad8", Key::Numpad8,
        "numpad9", Key::Numpad9,
        "f1", Key::F1,
        "f2", Key::F2,
        "f3", Key::F3,
        "f4", Key::F4,
        "f5", Key::F5,
        "f6", Key::F6,
        "f7", Key::F7,
        "f8", Key::F8,
        "f9", Key::F9,
        "f10", Key::F10,
        "f11", Key::F11,
        "f12", Key::F12,
        "f13", Key::F13,
        "f14", Key::F14,
        "f15", Key::F15,
        "pause", Key::Pause
    );
#pragma endregion

    Lua.script_file(path);
    // ObjectVector a = lua["a"];
    return;
}

void luaRun(std::string code) {
    Lua.script(code);
}
