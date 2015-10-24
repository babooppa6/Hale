#ifndef TLUA_TABLE_H
#define TLUA_TABLE_H

#include <functional>

#include <lua.hpp>
#include "tlua_reference.h"
#include "tlua_helpers.h"

namespace tlua {

    struct table : public typed_reference<LUA_TTABLE>
    {
        static table create(lua_State *L, int na, int nr)
        {
            lua_createtable(L, na, nr);
            table t(L, -1);
            lua_pop(L, 1);
            return t;
        }

        static table createMetatable(lua_State *L, const char *name)
        {
            if (luaL_newmetatable(L, name) == 0) {
                return table();
            }

            table t(L, -1);
            lua_pop(L, 1);
            return t;
        }


        table()
        {}

        table(lua_State *L, int index) :
            typed_reference<LUA_TTABLE>(L, index)
        {}

        template<typename T>
        T get(const char *k) {
            push(L);
            lua_pushstring(L, k);
            lua_gettable(L, -2);

            T result = pop<T>(L);
            lua_pop(L, 1);
            return result;
        }

        template<typename T>
        void set(const char *k, T& v) {
            table::push(L);
            lua_pushstring(L, k);
            tlua::push(L, v);
            lua_settable(L, -3);
            lua_pop(L, 1);
        }

        template<typename T>
        void setv(const char *k, T v) {
            table::push(L);
            lua_pushstring(L, k);
            tlua::push(L, v);
            lua_settable(L, -3);
            lua_pop(L, 1);
        }

        template<typename T>
        void set(int k, T& v) {
            table::push(L);
            lua_pushinteger(L, k);
            tlua::push(L, v);
            lua_settable(L, -3);
            lua_pop(L, 1);
        }


        template <typename T>
        void append(T &v) {
            table::push(L);
            // get and push the index at end of the table
            size_t i = lua_objlen(L, -1) + 1;
            lua_pushinteger(L, i);
            // push the value
            tlua::push(L, v);
            // set index and value
            lua_settable(L, -3);
            // luautil_stack_dump(L);
            lua_pop(L, 1);
        }

        void set(const luaL_Reg *c_functions) {
            push(L);
            luaL_setfuncs(L, c_functions, 0);
            lua_pop(L, 1);
        }

        int typeOf(const char *k) {
            push(L);
            lua_pushstring(L, k);
            lua_gettable(L, -2);
            int r = lua_type(L, -1);
            lua_pop(L, 2);
            return r;
        }

        /// Walks the keys in the table.
        void forEach(std::function<void(const char* key, int type)> f) const
        {
            push(L);
            lua_pushnil(L);

            while (lua_next(L, -2) != 0) {
                if (lua_type(L, -2) == LUA_TSTRING) {
                    const char* key = lua_tostring(L, -2);
                    auto type = lua_type(L, -1);
                    f(key, type);
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }

        // template<T>
        // T get(int index) {}
    };

    DEFINE_TYPE_TEMPLATE_FOR(table,     , return table(L, n),           return v.push(L));
}

#endif // TLUA_TABLE_H

