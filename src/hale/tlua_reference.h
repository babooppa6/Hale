#ifndef TLUA_REFERENCE_H
#define TLUA_REFERENCE_H

#include <QDebug>

#include <lua.hpp>
#include "luautil.h"
#include "tlua_value.h"

namespace tlua {

    struct reference
    {
        reference() :
            L(NULL),
            ref(LUA_NOREF)
        {}

        reference(lua_State *L, int index) : L(L) {
            init(L, index);
        }

        virtual ~reference() {
            if (ref != LUA_NOREF) {
                // qDebug() << "[REF] Unref" << ref << LuaType;
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
                ref = LUA_NOREF;
                L = NULL;
            }
        }

        reference(const reference &other) :
            ref(LUA_NOREF),
            L(NULL)
        {
            *this = other;
        }

        void init(lua_State *L, int index) {
            Q_ASSERT(L);
            if (!checkType(L, index)) {
                ref = LUA_NOREF;
                reference::L = NULL;
            } else {
                reference::L = L;
                lua_pushvalue(L, index);
                ref = luaL_ref(L, LUA_REGISTRYINDEX);
                Q_ASSERT(ref != LUA_NOREF);
                if (ref == LUA_REFNIL) {
                    ref = LUA_NOREF;
                }
//                Q_ASSERT(ref != LUA_REFNIL);
            }
        }

        reference &operator =(const reference &other) {
            int old_ref = ref;
            lua_State *old_L = L;

            ref = LUA_NOREF;
            L = NULL;

            if (!other.empty()) {
                other.push(other.L);
                if (!checkType(other.L, -1)) {
                    lua_pop(other.L, -1);
                } else {
                    L = other.L;
                    ref = luaL_ref(L, LUA_REGISTRYINDEX);
                    // qDebug() << "[REF] Ref" << ref << LuaType << "copy of" << other.ref;
                }
            }

            if (old_ref != LUA_NOREF) {
                // qDebug() << "[REF] Unref" << old_ref;
                luaL_unref(old_L, LUA_REGISTRYINDEX, old_ref);
            }
            return *this;
        }

        int push(lua_State *L2) const {
            if (ref != LUA_NOREF) {
                Q_ASSERT(L == L2);
                lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            } else {
                lua_pushnil(L2);
            }
            return 1;
        }

        bool empty() const {
            return L == NULL || ref == LUA_NOREF;
        }

        virtual bool checkType(lua_State *L, int i) {
            Q_UNUSED(L);
            Q_UNUSED(i);
            return true;
        }

        lua_State *L;
        int ref;
    };

    template<int LuaType>
    struct typed_reference : public reference
    {
        typed_reference() {}
        typed_reference(lua_State *L, int index) : reference() {
            init(L, index);
        }

        bool checkType(lua_State *L, int i) {
            int t = lua_type(L, i);
            if (t != LuaType) {
                qWarning() << __FUNCTION__ << "Reference is supposed to be of type" << LuaType;
                // Q_ASSERT_X(t == LuaType, __FUNCTION__, "Reference was not of required type.");
                return false;
            }
            return true;
        }
    };

    DEFINE_TYPE_TEMPLATE_FOR(reference, , return reference(L, n),       return v.push(L));
}

#endif // TLUA_REFERENCE_H

