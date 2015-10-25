#ifndef LUA_SCOPEPATH_H
#define LUA_SCOPEPATH_H

#include "tlua.h"

class ScopePath;
struct lua_State;

extern tlua::userdata lua_scopepath_wrap(lua_State *L, ScopePath *scope_path);
extern ScopePath *lua_scopepath_unwrap(lua_State *L, int i);

extern int luascopepath_push(lua_State *L, ScopePath *scope_path);
extern int lua_scopepath(lua_State *L);

#endif // LUA_SCOPEPATH_H

