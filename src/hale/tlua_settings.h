#ifndef TLUA_SETTINGS_H
#define TLUA_SETTINGS_H

#include <lua.hpp>
#include <functional>

namespace tlua {

struct settings_t
{
    std::function<bool(lua_State *L, int na, int nr)> pcall;
};

extern settings_t settings;

}

#endif // TLUA_SETTINGS_H
