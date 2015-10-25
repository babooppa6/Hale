#include "tlua.h"
#include "document.h"
#include "lua_document.h"

static int
luadoc_path(lua_State *L)
{
    auto document = tlua::read<Document*>(L, 1);
    return tlua::push<QString>(L, document->path());
}

static int
luadoc_hasPath(lua_State *L)
{
    auto document = tlua::read<Document*>(L, 1);
    return tlua::pushv<bool>(L, !document->path().isEmpty());
}


int lua_document(lua_State *L)
{
    static const struct luaL_Reg
    f[] = {
        {"path", 			luadoc_path},
        {"hasPath", 		luadoc_hasPath},
    };

    tlua::table metatable(tlua::table::createMetatable(L, tlua::value<Document*>::className()));
    metatable.set("__index", metatable);
    metatable.set(f);

    return metatable.push(L);
}
