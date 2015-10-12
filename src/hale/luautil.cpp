#include <QString>
#include <QObject>
#include <QDebug>

#include "luautil.h"
#include "luaobject.h"

void luautil_stack_dump(lua_State *L)
{
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


void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
    luaL_checkstack(L, nup, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -nup);
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_setfield(L, -(nup + 2), l->name);
    }
    lua_pop(L, nup);  /* remove upvalues */
}


void luautil_set_class(lua_State *L, const char *class_name, const luaL_Reg *class_functions)
{
    // MyTable
    luaL_newmetatable(L, class_name);
    // MyTable MyTable
    lua_pushvalue(L, -1);
    // MyTable.__index = MyTable
    lua_setfield(L, -2, "__index");
    // MyTable
    luaL_setfuncs(L, class_functions, 0);
}


quint32 __lutil_hexchar_to_int(char c)
{
    switch (c)
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return c - '0';
    case 'a': case 'b': case 'c':
    case 'd': case 'e': case 'f':
        return 0xA + c - 'a';
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return 0xA + c - 'A';
    }

    return 0;
}


int luautil_insistglobal(lua_State *L, const char *k)
{
    lua_getglobal(L, k);

    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1); // Pop the non-table.
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, k);
    }

    return 1;
}


//void luautil_refset(lua_State *L, LuaObject *object)
//{
//    Q_ASSERT(object->luaInstance() == LUA_REFNIL);
//    Q_ASSERT(object->luaState() == NULL);

//    // Duplicate top of the stack.
//    lua_pushvalue(L, -1);
//    // Make a reference to the top of the stack (and pop it).
//    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
//    // Set it to the object.
//    object->setLuaInstance(ref, L);
//}


//bool luautil_refget(lua_State *L, LuaObject *object)
//{
//    int ref = object->luaInstance();
//    if (ref != LUA_REFNIL) {
//        Q_ASSERT(object->luaState() == L);
//        if (object->luaState() == L) {
//            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
//            return true;
//        }
//    }
//    return false;
//}


QString luautil_checkstring(lua_State *L, int n)
{
    size_t str_l;
    const char *str = luaL_checklstring(L, n, &str_l);
    if (str == NULL) {
        luaL_error(L, LERROR_PARAM_N_EXPECTED_TYPE(n, "string"));
    }
    return QString(str);
}


//int luautil_wrap(lua_State *L, QObject *object)
//{
//    if (object == NULL) {
//        lua_pushnil(L);
//        return 1;
//    }

//    // if (!object->property("lua").isValid()) {
//    //     lua_pushnil(L);
//    //     return 1;
//    // }

//    auto lua_object = dynamic_cast<LuaObject*>(object);
//    Q_ASSERT(lua_object);
//    if (!lua_object) {
//        lua_pushnil(L);
//        return 1;
//    }


//    if (!lua_object->lua_self.empty()) {
//        return lua_object->lua_self.push(L);
//    }

//    int r = lua_object->lua_wrap(L);
//    if (r == 0) {
//        lua_pushnil(L);
//        return 1;
//    }

//    Q_ASSERT(r == 1);

//    return r;
//}


//void luautil_add_constant(lua_State *L, const char *name, int value)
//{
//    lua_pushnumber(L, value);
//    lua_setfield(L, -2, name);
//}
