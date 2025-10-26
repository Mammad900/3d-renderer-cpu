#ifndef __LUA_H__
#define __LUA_H__

#include <string>

void lua();
void luaOnFrame();
void luaDestroy();
void luaRun(std::string code);

#endif /* __LUA_H__ */
