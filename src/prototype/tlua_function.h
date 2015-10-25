#ifndef TLUA_FUNCTION_H
#define TLUA_FUNCTION_H

#include <functional>
#include <memory>
#include <lua.hpp>
#include "luautil.h"
#include "tlua_value.h"

#include "tlua_reference.h"
#include "tlua_settings.h"

namespace tlua {

struct function : typed_reference<LUA_TFUNCTION>
{
    function () {}
    function(lua_State *L, int index) :
        typed_reference<LUA_TFUNCTION>(L, index)
    {}

    bool invoke(int na, int nr) {
        push(L);
        // functions has to be before the arguments.
        if (na > 0) {
            lua_insert(L, -na-1);
        }
        return settings.pcall(L, na, nr);
    }
};

DEFINE_TYPE_TEMPLATE_FOR(function,  , return function(L, n),        return v.push(L));

}

#endif // TLUA_FUNCTION_H

