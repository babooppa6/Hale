#include "precompiled.h"
#include "luaengine.h"

#include "tlua.h"

#include "lua_app.h"
#include "lua_scopepath.h"
#include "lua_document.h"
#include "lua_documentmodel.h"
#include "lua_consolemodel.h"
#include "lua_panel.h"

// Handle lua_atpanic
// http://noahdesu.github.io/2013/01/24/custom-lua-panic-handler.html

int panic(lua_State *L)
{
    // Report error!
    qDebug() << "[LUA.PANIC]" << __FUNCTION__ << lua_tostring(L, -1);
    Q_ASSERT(false && "Panic");
    // _CrtDbgBreak();
    return 0;
}

LuaEngine *LuaEngine::instance = NULL;

LuaEngine::LuaEngine(Application *) :
    L(NULL)
{
    Q_ASSERT(instance == NULL);
    instance = this;
    m_log = new Document(this);
}

LuaEngine::~LuaEngine()
{
    lua_close(L);
    L = 0;
}

void LuaEngine::init()
{
    L = luaL_newstate();
    lua_atpanic(L, panic);

    luaL_openlibs(L);

    tlua::lib::set(L, lua_scopepath, "scopepath");
    tlua::lib::set(L, lua_documentmodel, "document");
    tlua::lib::set(L, lua_document, "documentbuffer");
    tlua::lib::set(L, lua_consolemodel, "console");
    tlua::lib::set(L, lua_panel, "panel");

    // Need to be done after lua_scopepath.
    tlua::lib::set(L, lua_app, "app");
}

bool LuaEngine::execute(const QString &path, int nr)
{
    int error = luaL_loadfile(L, path.toUtf8());
    if (error) {
        printError(error);
        lua_pop(L, 1);
        return false;
    }

    return pcall(L, 0, nr);
}

bool LuaEngine::pcall(lua_State *L, int na, int nr)
{
    int error = lua_pcall(L, na, nr, 0);
    if (error) {
        printError(error);
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void LuaEngine::printError(int error)
{
    QString message;
    switch (error)
    {
    case LUA_ERRRUN:
        message.append("[ERRRUN]");
        break;
    case LUA_ERRMEM:
        message.append("[ERRMEM]");
        break;
    case LUA_ERRERR:
        message.append("[ERRERR]");
        break;
    }

    message.append(lua_tostring(L, -1));
    qDebug() << message;
    print(message);
}

void LuaEngine::print(const QString &message)
{
    DocumentEdit e = m_log->edit(NULL);
    if (!m_log->isEmpty()) {
        m_log->appendText(e, "\n");
    }
    m_log->appendText(e, message);
}
