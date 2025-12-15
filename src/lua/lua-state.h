#ifndef __LUA_STATE_H__
#define __LUA_STATE_H__

#include <filesystem>
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

extern sol::state Lua;

template<typename T>

T valueFromObject(sol::object obj, T def = T{});
std::filesystem::path get_calling_script_path(sol::state_view& lua);
void luaImGui();
void luaSimples();
void luaScene();
void luaWindow();
void luaObject();
void luaCamera();
void luaMaterial();
void luaMesh();
void luaLights();
void luaTextures();

#endif /* __LUA_STATE_H__ */
