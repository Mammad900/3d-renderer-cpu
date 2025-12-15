#include "lua-state.h"
#include "../texture.h"
#include "../textureFiltering.h"
#include "../tinyTexture.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

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
        "as_texture", [](std::shared_ptr<ImageTexture<T>>& l) -> std::shared_ptr<Texture<T>> { return l; },
        "save_to_file", [](sol::this_state s, std::shared_ptr<ImageTexture<T>>& tex, std::string path) {
            sol::state_view lua(s);
            return tex->saveToImage().saveToFile(get_calling_script_path(lua) / path);
        }
    );

    Lua.new_usertype<SliceTexture<T>>("Slice"+name+"Texture",
        sol::meta_function::construct, [](shared_ptr<Texture<T>> texture, sol::table scale, sol::table offset) {
            return std::make_shared<SliceTexture<T>>(
                texture,
                Vector2f(scale.get<float>(1), scale.get<float>(2)),
                Vector2f(offset.get<float>(1), offset.get<float>(2))
            );
        },
        "scale", &SliceTexture<T>::scale,
        "offset", &SliceTexture<T>::offset,
        "texture", &SliceTexture<T>::texture,
        "as_texture", [](std::shared_ptr<SliceTexture<T>>& l) -> std::shared_ptr<Texture<T>> { return l; }
    );
    name[0] = tolower(name[0]);
    Lua.set_function("slice_"+name+"_texture", [](shared_ptr<Texture<T>> texture, sol::table nt) {        
        Vector2u n(nt.get<unsigned int>(1), nt.get<unsigned int>(2));
        sol::table slices = Lua.create_table();
        Vector2f scale(1.0f / n.x, 1.0f / n.y);

        for (unsigned int y = 0; y < n.y; ++y) {
            for (unsigned int x = 0; x < n.x; ++x) {
                Vector2f offset(x * scale.x, y * scale.y);
                slices[y * n.x + x + 1] = std::make_shared<SliceTexture<T>>(texture, scale, offset);
            }
        }

        return slices;
    });
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

void luaTextures() {
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
        "as_texture", [](std::shared_ptr<TinyImageTexture>& l) -> std::shared_ptr<Texture<Color>> { return l; },
        "save_to_file", [](sol::this_state s, std::shared_ptr<TinyImageTexture>& tex, std::string path) {
            sol::state_view lua(s);
            return tex->image.saveToFile(get_calling_script_path(lua) / path);
        }
    );

    Lua.new_usertype<EnvironmentMap>("EnvironmentMap", 
        sol::no_constructor
    );
    Lua.new_usertype<SolidEnvironmentMap>("SolidEnvironmentMap",
        sol::meta_function::construct, [](sol::object value) {
            return std::make_shared<SolidEnvironmentMap>(valueFromObject<Color>(value));
        },
        "color", &SolidEnvironmentMap::value,
        "as_environment_map", [](std::shared_ptr<SolidEnvironmentMap>& l) -> std::shared_ptr<EnvironmentMap> { return l; }
    );
    Lua.new_usertype<PanoramaMap>("PanoramaMap",
        sol::meta_function::construct, [](shared_ptr<Texture<Color>> texture) {
            return std::make_shared<PanoramaMap>(texture);
        },
        "texture", &PanoramaMap::texture,
        "as_environment_map", [](std::shared_ptr<PanoramaMap>& l) -> std::shared_ptr<EnvironmentMap> { return l; }
    );
    Lua.new_usertype<AtlasCubeMap>("AtlasCubeMap",
        sol::meta_function::construct, [](shared_ptr<Texture<Color>> texture) {
            return std::make_shared<AtlasCubeMap>(texture);
        },
        "texture", &AtlasCubeMap::texture,
        "as_environment_map", [](std::shared_ptr<AtlasCubeMap>& l) -> std::shared_ptr<EnvironmentMap> { return l; }
    );
    Lua.new_usertype<CubeMap>("CubeMap",
        sol::meta_function::construct, [](sol::table textures) {
            return std::make_shared<CubeMap>(std::array<shared_ptr<Texture<Color>>, 6>{
                textures[1], textures[2], textures[3],
                textures[4], textures[5], textures[6],
            });
        },
        // "textures", &AtlasCubeMap::textures,
        "as_environment_map", [](std::shared_ptr<CubeMap>& l) -> std::shared_ptr<EnvironmentMap> { return l; }
    );
}