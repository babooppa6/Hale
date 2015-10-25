#include "application.h"

#include "panel.h"
#include "documentmodel.h"

#include "tlua.h"
#include "lua_panel.h"

static int
lua_panel_open(lua_State *L)
{
    // lua_panel_t *w = lua_panel_read(L, 1);
    auto panel = tlua::read<Panel*>(L, 1);

    // TODO: This should be any of the models, but we don't have inheritance handling in Lua yet.
    // TODO: We need an inheritance handling in Lua somehow.
    auto model = tlua::read<DocumentModel*>(L, 2);

    panel->setModel(model);
    return 0;
}

static int
lua_panel_focus(lua_State *L)
{
    auto panel = tlua::read<Panel*>(L, 1);
    Application::instance()->focusPanel(panel);
    return 0;
}

static int
lua_panel_close(lua_State *L)
{
    auto panel = tlua::read<Panel*>(L, 1);
    Application::instance()->closePanel(panel);
    return 0;
}

int
lua_panel(lua_State *L)
{
    static const struct luaL_Reg
    f[] = {
        {"open", lua_panel_open},
        {"focus", lua_panel_focus},
        {"close", lua_panel_close},
        {NULL, NULL}
    };

    tlua::table metatable(tlua::table::createMetatable(L, tlua::value<Panel*>::className()));
    metatable.set("__index", metatable);
    metatable.set(f);

    // luautil_set_class(L, LUA_CLASS_CONSOLEMODEL, f);

    return metatable.push(L);
}
