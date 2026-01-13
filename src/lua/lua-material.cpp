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
            shared_ptr<Texture<Color>> diffuse               = properties.get_or<shared_ptr<Texture<Color>>>("diffuse"               , nullptr);
            shared_ptr<Texture<Color>> specular              = properties.get_or<shared_ptr<Texture<Color>>>("specular"              , nullptr);
            shared_ptr<Texture<Color>> tint                  = properties.get_or<shared_ptr<Texture<Color>>>("tint"                  , nullptr);
            shared_ptr<Texture<Color>> emissive              = properties.get_or<shared_ptr<Texture<Color>>>("emissive"              , nullptr);
            shared_ptr<Texture<Color>> environmentReflection = properties.get_or<shared_ptr<Texture<Color>>>("environment_reflection", nullptr);
            shared_ptr<Texture<Vec3 >> normalMap             = properties.get_or<shared_ptr<Texture<Vec3 >>>("normal_map"            , nullptr);

            auto mat = std::make_shared<PhongMaterial>(
                diffuse               ? diffuse               : std::make_shared<SolidTexture<Color>>(Color{1,1,1,1}),
                specular              ? specular              : std::make_shared<SolidTexture<Color>>(Color{0,0,0,0}),
                tint                  ? tint                  : std::make_shared<SolidTexture<Color>>(Color{0,0,0,0}),
                emissive              ? emissive              : std::make_shared<SolidTexture<Color>>(Color{0,0,0,0}),
                environmentReflection ? environmentReflection : std::make_shared<SolidTexture<Color>>(Color{0,0,0,0}),
                normalMap,
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
        "diffuse", &PhongMaterial::diffuse,
        "specular", &PhongMaterial::specular,
        "tint", &PhongMaterial::tint,
        "emissive", &PhongMaterial::emissive,
        "normal_map", sol::property(
            [](PhongMaterial &self) { return self.normalMap; },
            [](PhongMaterial &self, shared_ptr<Texture<Vec3>> value) { self.normalMap = value; self.needsTBN = value != nullptr; }
        ),
        "volume_front", &PhongMaterial::volumeFront,
        "volume_back", &PhongMaterial::volumeBack,
        "environment_reflection", &PhongMaterial::environmentReflection,
        "as_material", [](shared_ptr<PhongMaterial> &c)-> shared_ptr<Material> { return c; }
    );

    Lua.new_usertype<EarthMaterial>("EarthMaterial",
        sol::base_classes, sol::bases<Material>(),
        sol::meta_function::construct, [](sol::table properties) {
            std::string name = properties.get_or<std::string>("name", "Earth");

            auto mat = std::make_shared<EarthMaterial>(name);
            materials.emplace_back(mat);

            // Terrain textures
            mat->terrainMat->diffuse   = properties.get_or("terrain_diffuse", mat->terrainMat->diffuse);
            mat->terrainMat->emissive  = properties.get_or("city_lights"    , mat->terrainMat->emissive);
            mat->terrainMat->normalMap = properties.get_or("normal_map"     , mat->terrainMat->normalMap);
            
            // Ocean textures
            mat->oceanMat->diffuse     = properties.get_or("ocean_diffuse"  , mat->oceanMat->diffuse);
            mat->oceanMat->specular    = properties.get_or("ocean_specular" , mat->oceanMat->specular);
            mat->oceanMask             = properties.get_or("ocean_mask"     , mat->oceanMask);

            // Cloud textures
            mat->cloudMat->diffuse     = properties.get_or("cloud_diffuse"  , mat->cloudMat->diffuse);
            mat->cloudTexture          = properties.get_or("cloud_texture"  , mat->cloudTexture);

            return mat;
        },
        "as_material", [](std::shared_ptr<EarthMaterial> &c) -> std::shared_ptr<Material> { return c; },

        // Expose texture handles for Lua access
        "terrain_diffuse", sol::property(
            [](EarthMaterial &self) { return self.terrainMat->diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.terrainMat->diffuse = tex; }
        ),
        "city_lights", sol::property(
            [](EarthMaterial &self) { return self.terrainMat->emissive; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.terrainMat->emissive = tex; }
        ),
        "ocean_diffuse", sol::property(
            [](EarthMaterial &self) { return self.oceanMat->diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.oceanMat->diffuse = tex; }
        ),
        "ocean_specular", sol::property(
            [](EarthMaterial &self) { return self.oceanMat->specular; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.oceanMat->specular = tex; }
        ),
        "cloud_diffuse", sol::property(
            [](EarthMaterial &self) { return self.cloudMat->diffuse; },
            [](EarthMaterial &self, std::shared_ptr<Texture<Color>> tex) { self.cloudMat->diffuse = tex; }
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