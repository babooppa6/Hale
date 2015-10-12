#define LUA_CLASS_CONSOLEMODEL "Console"

#include <lua.hpp>
#include "luautil.h"
#include "lua_consolemodel.h"
#include "lua_scopepath.h"
#include "application.h"

#include "pathcompletioncontroller.h"
#include "document.h"
#include "documentmodel.h"
#include "consolemodel.h"

static int
lua_consolemodel_line(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);

    QByteArray text(console->commandView()->text().toUtf8());
    lua_pushstring(L, text.constData());
    return 1;
}

static int
lua_consolemodel_cursorOffset(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);

    auto view = console->commandView();
    int p = view->document()->blockPositionToOffset(view->cursor()->position()) + 1;
    int a = view->document()->blockPositionToOffset(view->cursor()->anchor()) + 1;

    lua_pushinteger(L, p);
    lua_pushinteger(L, a);

    return 2;
}

static int
lua_consolemodel_completeSegment(lua_State *L)
{
    Q_UNUSED(L);
//    lua_consolemodel_t *lcon = lua_consolemodel_pop(L, 1);
//    int begin = luaL_checkinteger(L, 1);
//    int end = luaL_checkinteger(L, 1);
    // TODO: pop completion table
    return 0;
}

static int
lua_consolemodel_clear(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    auto view = console->commandView();
    auto e = view->edit();
    view->clear(e);
    return 0;
}

static int
lua_consolemodel_controller(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    console->lua_controller = tlua::table(L, 2);
    return 0;
}

static int
lua_consolemodel_execute(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    console->execute();
    return 0;
}

static int
lua_consolemodel_complete(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    tlua::table command(L, 2);
//    auto scope_path_ptr = lua_scopepath_unwrap(L, 3);
//    ScopePath scope_path(*scope_path_ptr);
    console->complete(command);
    return 0;
}

static int
lua_consolemodel_set(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    QString text(tlua::read<QString>(L, 2));
    console->set(text);
    Application::instance()->setActiveModel(console);

    return 0;
}

static int
lua_consolemodel_beginCompletion(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    int begin = tlua::read<int>(L, 2) - 1;
    int end = tlua::read<int>(L, 3) - 1;
    if (lua_isstring(L, 4)) {
        QString built_in(tlua::read<QString>(L, 4));
        if (built_in == "path") {
            auto view = console->commandView();
            view->beginCompletion(begin, end, new PathCompletionController(view, view));
        }
    }
    return 0;
}

static int
lua_consolemodel_endCompletion(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    console->commandView()->endCompletion();
    return 0;
}

static int
lua_consolemodel_context(lua_State *L)
{
    auto console = tlua::read<ConsoleModel*>(L, 1);
    return console->lua_context_scope.push(L);
}

int
lua_consolemodel(lua_State *L)
{
    static const struct luaL_Reg
    f[] = {
        {"line", lua_consolemodel_line},
        {"cursorOffset", lua_consolemodel_cursorOffset},
        {"clear", lua_consolemodel_clear},
        {"completeSegment", lua_consolemodel_completeSegment},
        {"controller", lua_consolemodel_controller},
        {"execute", lua_consolemodel_execute},
        {"beginCompletion", lua_consolemodel_beginCompletion},
        {"endCompletion", lua_consolemodel_endCompletion},
        {"complete", lua_consolemodel_complete},
        {"set", lua_consolemodel_set},
        {"context", lua_consolemodel_context},
        {NULL, NULL}
    };

    tlua::table metatable(tlua::table::createMetatable(L, tlua::value<ConsoleModel*>::className()));
    metatable.set("__index", metatable);
    metatable.set(f);

    // luautil_set_class(L, LUA_CLASS_CONSOLEMODEL, f);

    return metatable.push(L);
}
