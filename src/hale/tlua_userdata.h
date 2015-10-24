#ifndef TLUA_USERDATA_H
#define TLUA_USERDATA_H

#include "tlua_reference.h"
#include "tlua_value.h"
#include "tlua_table.h"

namespace tlua
{
    struct userdata : public typed_reference<LUA_TUSERDATA>
    {
        userdata() {}
        userdata(lua_State *L, int index) :
            typed_reference<LUA_TUSERDATA>(L, index)
        {}

        table metatable(lua_State *L) {
            push(L);
            if (lua_getmetatable(L, -1) == 0) {
                lua_pop(L, 1);
                return table();
            }
            table metatable(L, -1);
            lua_pop(L, 2);
            return metatable;
        }
    };

    DEFINE_TYPE_TEMPLATE_FOR(userdata,  , return userdata(L, n),        return v.push(L));
}

#endif // TLUA_USERDATA_H

