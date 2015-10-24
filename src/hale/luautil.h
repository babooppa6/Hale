#ifndef __LUA_UTIL_H__
#define __LUA_UTIL_H__

#include <QString>

struct lua_State;
struct luaL_Reg;

class QObject;
class LuaObject;

#define LERROR_PARAM_N_EXPECTED(n) #n " parameters expected"
#define LERROR_PARAM_N_EXPECTED_TYPE(n, type) "parameter " #n " has to be of type " type
#define LERROR_NOT_FOUND(n) "`" #n "` was not found"
#define LERROR_F_NOT_FOUND "%s was not found"

extern void luautil_stack_dump(lua_State *L);
extern void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

extern void luautil_set_class(lua_State *L, const char *class_name, const struct luaL_Reg *class_functions);

extern int luautil_insistglobal(lua_State *L, const char *k);

extern QString luautil_checkstring(lua_State *L, int n);

// extern void luautil_add_constant(lua_State *L, const char *name, int value);
// extern int luautil_wrap(lua_State *L, QObject *object);

#endif // __LUA_UTIL_H__
