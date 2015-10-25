#ifndef TLUA_MODULE_H
#define TLUA_MODULE_H

#include <lua.hpp>

namespace tlua {

    struct lib
    {
        /// Initializes the module by calling the function f, then adds the result
        /// to package.loaded[name].
        static int
        set(lua_State *L, lua_CFunction f, const char *name)
        {
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "loaded");
            // NOTE: In Lua 5.1, loadlib.c, ll_require always returns one value.
            int r = f(L);
            Q_ASSERT(r == 1);
            lua_setfield(L, -2, name);
            lua_pop(L, 2);
            return 0;
        }

        /// Adds the function to package.preload[name]. The function is not called,
        /// but anytime the user requires the module, Lua first checks for function
        /// in package.preload.
        static int
        def(lua_State *L, lua_CFunction f, const char *name)
        {
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "preload");
            lua_pushcfunction(L, f);
            lua_setfield(L, -2, name);
            lua_pop(L, 2);
            return 0;
        }
    };

}

#endif // TLUA_MODULE_H

