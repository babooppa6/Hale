#ifndef TLUA_VALUE_H
#define TLUA_VALUE_H

namespace tlua {

//
// default
//

template<typename T>
struct value
{
    static_assert(
        sizeof(T) == -1,
        "Parameter to Value is not supported"
    );
};

//
// nullptr_t
//

template<>
struct value<std::nullptr_t> {
    static inline
    std::nullptr_t read(lua_State *L, int n) {
        luaL_checktype(L, n, LUA_TNIL);
        return nullptr;
    }

    static inline
    int push(lua_State* L, std::nullptr_t) {
        lua_pushnil(L);
        return 1;
    }
};

#define DEFINE_TYPE_TEMPLATE_FOR(type, spec, readcommand, pushcommand)\
    template<spec>\
    struct value<type>\
    {\
        static inline\
        type read(lua_State *L, int n) {\
            readcommand;\
        }\
\
        static inline\
        int push(lua_State *L, type &v) {\
            pushcommand;\
        }\
    }


DEFINE_TYPE_TEMPLATE_FOR(bool,  , return lua_toboolean(L, n) != 0,       lua_pushboolean(L, v); return 1);
DEFINE_TYPE_TEMPLATE_FOR(int,   , return lua_tointeger(L, n),            lua_pushinteger(L, v); return 1);
DEFINE_TYPE_TEMPLATE_FOR(float, , return lua_tonumber(L, n),             lua_pushnumber(L, v); return 1);
DEFINE_TYPE_TEMPLATE_FOR(double,, return lua_tonumber(L, n),             lua_pushnumber(L, v); return 1);

DEFINE_TYPE_TEMPLATE_FOR(const char*,   , return lua_tostring(L, n),           lua_pushstring(L, v); return 1);
DEFINE_TYPE_TEMPLATE_FOR(char*,         , return (char*)lua_tostring(L, n),    lua_pushstring(L, v); return 1);

//
// userdata
//

//template<>
//struct value<userdata> {
//    static inline
//    userdata read(lua_State *L, int n) {
//        userdata res(L, n);
//        return res;
//    }

//    static inline
//    int push(lua_State*, const userdata &r) {
//        r.push();
//        return 1;
//    }
//};

////
//// table
////

//template<>
//struct value<table> {
//    static inline
//    table read(lua_State *L, int n) {
//        table res(L, n);
//        return res;
//    }

//    static inline
//    int push(lua_State*, const table &t) {
//        t.push();
//        return 1;
//    }
//};

////
//// function
////

//template<>
//struct value<function> {
//    static inline
//    function read(lua_State *L, int n) {
//        function res(L, n);
//        return res;
//    }

//    static inline
//    int push(lua_State*, const function &f) {
//        f.push();
//        return 1;
//    }
//};

}

#endif // TLUA_VALUE_H

