#ifndef TLUA_HELPERS_H
#define TLUA_HELPERS_H

#include <cstddef>
#include "tlua_value.h"

namespace tlua
{
    //
    // Push helper.
    //

    template <typename T>
    static inline
    int push(lua_State* L, T &v) {
        Q_ASSERT(L);
        return value<T>::push(L, v);
    }

    template <typename T>
    static inline
    int pushv(lua_State* L, T v) {
        Q_ASSERT(L);
        return value<T>::push(L, v);
    }

    static int push_nil(lua_State *L) {
        Q_ASSERT(L);
        return pushv<std::nullptr_t>(L, nullptr);
    }

    //
    // Read helper
    //

    template <typename T>
    static inline
    T read(lua_State *L, int n) {
        Q_ASSERT(L);
        return value<T>::read(L, n);
    }

    //
    // Pop helper.
    //

    template <typename T> static inline
    T pop(lua_State* L) {
        Q_ASSERT(L);
        T ret = value<T>::read(L, -1);
        lua_pop(L, 1);
        return ret;
    }

    //
    // Stack dump
    //

    #include <QString>
    static void dump(lua_State *L)
    {
        Q_ASSERT(L);
        QString line;
        int i;
        int top = lua_gettop(L);
        Q_ASSERT(top >= 0);
        if (top == 0) {
            qDebug() << "Stack is empty";
            return;
        }

        for (i = 1; i <= top; i++) { /* repeat for each level */
            int t = lua_type(L, i);
            const void *p = lua_topointer(L, i);

            line.clear();
            line.append(QString("%1. %2. ").arg(i, 2).arg(-(top-i)-1, 2));
            // qDebug("% 2d. % 2d. ", i, -(top-i)-1);
            switch (t) {
            case LUA_TSTRING: { /* strings */
                line.append(QString("'%1'").arg(lua_tostring(L, i)));
                // qDebug("'%s'", lua_tostring(L, i));
                break;
            }
            case LUA_TBOOLEAN: { /* booleans */
                line.append(QString(lua_toboolean(L, i) ? "true" : "false"));
                // qDebug(lua_toboolean(L, i) ? "true" : "false");
                break;
            }
            case LUA_TNUMBER: { /* numbers */
                line.append(QString::number(lua_tonumber(L, i)));
                // qDebug("%g", lua_tonumber(L, i));
                break;
            }
            default: { /* other values */
                line.append(QString("%1 [%2]").arg(lua_typename(L, t)).arg((uintptr_t)p, 8, 16));
                // qDebug("%s %x", lua_typename(L, t), (uintptr_t)p);
                break;
            }
            }
            qDebug().noquote() << line;
        }
        qDebug("---\n"); /* end the listing */
    }
}

#endif // TLUA_HELPERS_H

