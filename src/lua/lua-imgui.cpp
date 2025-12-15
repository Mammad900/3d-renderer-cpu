#include "lua-state.h"
#include "sol/sol_ImGui.h"

void luaImGui() {
    sol_ImGui::Init(Lua);
}