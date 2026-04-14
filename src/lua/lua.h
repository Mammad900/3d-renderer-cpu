#ifndef __LUA_H__
#define __LUA_H__

#include <optional>
#include <string>

void lua(std::string path);
void luaOnFrame();
void luaDestroy();
std::optional<std::string> luaRun(std::string code);

#endif /* __LUA_H__ */
