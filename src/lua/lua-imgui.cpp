#include "lua-state.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Warray-bounds"
#elif __clang__
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

#include "sol/sol_ImGui.h"

void luaImGui() {
    sol_ImGui::Init(Lua);
}