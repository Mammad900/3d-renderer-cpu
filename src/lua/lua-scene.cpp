#include "lua-state.h"
#include "../data.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaScene() {
        Lua.new_usertype<Scene>("Scene",
        sol::meta_function::construct, []() { 
            std::shared_ptr<Scene> scene = std::make_shared<Scene>(); 
            scenes.push_back(scene);
            return scene;
        },
        "name", &Scene::name,
        "sky_box", &Scene::skyBox,
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
        "bilinear_shadow_filtering", &Scene::bilinearShadowFiltering,
        "shadow_bias", &Scene::shadowBias,
        "wire_frame", &Scene::wireFrame,
        "full_bright", &Scene::fullBright,
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
}