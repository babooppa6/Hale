#ifndef LUAOBJECT_H
#define LUAOBJECT_H

struct lua_State;

#include <QObject>
#include <QMetaObject>
#include "tlua.h"

class LuaObject
{
public:
    LuaObject();
    virtual ~LuaObject();

    tlua::userdata lua_self;

    template<class T>
    struct wrapper_t {
        T* o;
    };
};

#define LUAOBJECT_DECLARE(T)\
namespace tlua {\
template<>\
struct value<T*> {\
\
    static inline const char *className() { return T::staticMetaObject.className(); }\
    static inline\
    T* read(lua_State *L, int n) {\
        auto w = (LuaObject::wrapper_t<T>*)luaL_checkudata(L, n, className());\
        if (w == NULL) {\
            luaL_error(L, QString("parameter %1 has to be of type").arg(n).arg(className()).toUtf8().constData());\
        }\
        return w->o;\
    }\
\
    static inline\
    int push(lua_State* L, T* object) {\
        if (object->lua_self.empty()) {\
            auto w = (LuaObject::wrapper_t<T>*)lua_newuserdata(L, sizeof(LuaObject::wrapper_t<T>));\
            w->o = object;\
            luaL_getmetatable(L, object->metaObject()->className());\
            lua_setmetatable(L, -2);\
            object->lua_self = tlua::userdata(L, -1);\
            return 1;\
        }\
        return object->lua_self.push(L);\
    }\
};\
}

namespace tlua {
template<>
struct value<QObject*> {

    static inline
    int push(lua_State *L, QObject *object) {
        auto lua_object = dynamic_cast<LuaObject*>(object);
        if (lua_object == NULL) {
            lua_pushnil(L);
            return 1;
        }

        if (lua_object->lua_self.empty()) {
            auto w = (LuaObject::wrapper_t<QObject>*)lua_newuserdata(L, sizeof(LuaObject::wrapper_t<QObject>));\
            w->o = object;\
            qDebug() << __FUNCTION__ << object->metaObject()->className();\
            luaL_getmetatable(L, object->metaObject()->className());\
            lua_setmetatable(L, -2);\
            lua_object->lua_self = tlua::userdata(L, -1);\
            return 1;
        }
        return lua_object->lua_self.push(L);
    }
};
}

#endif // LUAOBJECT_H
