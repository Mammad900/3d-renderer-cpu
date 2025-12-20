#include "lua-state.h"
#include "../material.h"
#include "../phongMaterial.h"
#include "../pbrMaterial.h"
#include "../earthMaterial.h"
#include "../gui.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaMaterial() {
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
            if(properties["environment_reflection"].valid())
                props.environmentReflection = valueFromObject<Color>(properties["environment_reflection"]);

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
        "diffuse", sol::property(
            [](PhongMaterial &self) { return self.mat.diffuse; },
            [](PhongMaterial &self, shared_ptr<Texture<Color>> value) { self.mat.diffuse = value; }
        ),
        "specular", sol::property(
            [](PhongMaterial &self) { return self.mat.specular; },
            [](PhongMaterial &self, shared_ptr<Texture<Color>> value) { self.mat.specular = value; }
        ),
        "tint", sol::property(
            [](PhongMaterial &self) { return self.mat.tint; },
            [](PhongMaterial &self, shared_ptr<Texture<Color>> value) { self.mat.tint = value; }
        ),
        "emissive", sol::property(
            [](PhongMaterial &self) { return self.mat.emissive; },
            [](PhongMaterial &self, shared_ptr<Texture<Color>> value) { self.mat.emissive = value; }
        ),
        "normal_map", sol::property(
            [](PhongMaterial &self) { return self.mat.normalMap; },
            [](PhongMaterial &self, shared_ptr<Texture<Vec3>> value) { self.mat.normalMap = value; self.needsTBN = value != nullptr; }
        ),
        "volume_front", &PhongMaterial::volumeFront,
        "volume_back", &PhongMaterial::volumeBack,
        "environment_reflection", sol::property(
            [](PhongMaterial &self) { return self.mat.environmentReflection; },
            [](PhongMaterial &self, Color value) { self.mat.environmentReflection = value; }
        ),
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
}