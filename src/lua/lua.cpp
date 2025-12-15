#include "lua.h"
#include "../color.h"
#include "../data.h"
#include "../vector3.h"
#include "lua-state.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/String.hpp>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <string>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

sol::state Lua;

void luaOnFrame() {
    sol::table callbacks = Lua["on_frame"];
    for (auto [_, val] : callbacks) {
        if(val.get_type() == sol::type::function) {
            sol::function fn = val;
            fn(timing.deltaTime);
        }
    }
    Lua["total_time"] = timing.totalTime;
}

void luaDestroy() {
    // onFrameCallbacks.clear(); // otherwise the lua functions outliving the lua state will cause a segfault.
}

template float valueFromObject<float>(sol::object obj, float def);
template Vec3 valueFromObject<Vec3>(sol::object obj, Vec3 def);
template Color valueFromObject<Color>(sol::object obj, Color def);

template<typename T>
T valueFromObject(sol::object obj, T def) {
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


void lua(std::string path) {
	Lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::math);

    luaSimples();
    luaScene();
    luaWindow();
    luaObject();
    luaCamera();
    luaMaterial();
    luaMesh();
    luaLights();
    luaTextures();

    Lua["on_frame"] = sol::table(Lua, sol::create);
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

    luaImGui();
    Lua.script_file(path);
    return;
}

void luaRun(std::string code) {
    Lua.script(code);
}
