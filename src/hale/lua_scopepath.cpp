#define LUA_CLASS_SCOPEPATH "ScopePath"

#include <lua.hpp>
#include "tlua.h"
#include "luaobject.h"
#include "configurationobserver.h"
#include "luautil.h"

#include "lua_scopepath.h"

#include "scopepath.h"
#include "application.h"

struct lua_scopepath_t {
    ScopePath *path;
};

tlua::userdata
lua_scopepath_wrap(lua_State *L, ScopePath *scope_path)
{
    lua_scopepath_t *m = (lua_scopepath_t*)lua_newuserdata(L, sizeof(lua_scopepath_t));
    m->path = scope_path;

    luaL_getmetatable(L, LUA_CLASS_SCOPEPATH);
    lua_setmetatable(L, -2);

    tlua::userdata lua_scope_path(L, -1);
    lua_pop(L, 1);

    return lua_scope_path;
}

ScopePath *
lua_scopepath_unwrap(lua_State *L, int i)
{
    auto path = (lua_scopepath_t*)luaL_checkudata(L, i, LUA_CLASS_SCOPEPATH);
    return path->path;
}


//int
//luascopepath_push(lua_State *L, ScopePath *)
//{
//    lua_createtable(L, 0, 0);
//    luaL_getmetatable(L, LUA_CLASS_SCOPEPATH);
//    lua_setmetatable(L, -2);

//    return 1;
//}

static int
luascopepath_top(lua_State *L)
{
    auto path = (lua_scopepath_t*)luaL_checkudata(L, 1, LUA_CLASS_SCOPEPATH);
    // const ScopePath *path = Application::instance()->scopePath();
    QString name(luautil_checkstring(L, 2));
    ConfigurationObserver *object = path->path->top(name);
    return tlua::push<QObject*>(L, object->qObject);
}

static int
luascopepath_dump(lua_State *L)
{
    auto path = (lua_scopepath_t*)luaL_checkudata(L, 1, LUA_CLASS_SCOPEPATH);
    QString dump(path->path->toString());
    return tlua::push<QString>(L, dump);
}

int
lua_scopepath(lua_State *L)
{
    luaL_Reg functions[] = {
        {"top", luascopepath_top},
        {"dump", luascopepath_dump},
        {NULL, NULL}
    };

    tlua::table metatable(tlua::table::createMetatable(L, LUA_CLASS_SCOPEPATH));
    metatable.set("__index", metatable);
    metatable.set(functions);

    return metatable.push(L);
}
