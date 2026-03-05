#include "lua-state.h"
#include "../postProcessing.h"
#include "../lib/stb_image.h"
#include "../lib/stb_image_write.h"
#include "sol/sol.hpp"
#include <SFML/System/Vector2.hpp>
#include <filesystem>
#include <string>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

void luaPostProcessing() {
    Lua.new_usertype<RenderedImage>("RenderedImage",
        "clip", sol::overload(
            [](RenderedImage &self, sol::object max) {
                return self.clip({}, valueFromObject<Color>(max));
            },
            [](RenderedImage &self, sol::object min, sol::object max) {
                return self.clip(valueFromObject<Color>(min), valueFromObject<Color>(max));
            }
        ),
        "bloom", &RenderedImage::bloom,
        "blur", &RenderedImage::blur,
        "downscale", static_cast<RenderedImage (RenderedImage::*)(uint)>(&RenderedImage::downscale), // there's two overloads, have to select one
        "tonemap", &RenderedImage::tonemap,
        "save", [](sol::this_state s, RenderedImage &self, std::string path) {
            sol::state_view lua(s);
            std::filesystem::path fullPath = get_calling_script_path(lua) / path;
            stbi_write_hdr(fullPath.c_str(), self.size.x, self.size.y, 4, &self.data[0].r);
        }
    );
    Lua["RenderedImage"]["load"] = [](sol::this_state s, std::string path) {
        sol::state_view lua(s);
        std::filesystem::path fullPath = get_calling_script_path(lua) / path;
        int x,y,n;
        float *data = stbi_loadf(fullPath.c_str(), &x, &y, &n, 4);
        RenderedImage res(Vector2u{(uint)x,(uint)y});
        std::memcpy(res.data.data(), data, sizeof(Color) * res.size.x * res.size.y);
        return res;
    };
}