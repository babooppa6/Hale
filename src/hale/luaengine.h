#ifndef LUAENGINE_H
#define LUAENGINE_H

#include <QObject>

#include <lua.hpp>

#include "document.h"

// @see https://github.com/torch/qtlua/tree/master/qtlua
// @see https://github.com/davidsiaw/luacppinterface#lua-object (5.2 only)
// @see https://github.com/vapourismo/luwra (5.1, lightweight, template-based)

class Application;

class LuaEngine : public QObject
{
    Q_OBJECT
public:
    static LuaEngine *instance;

    LuaEngine(Application *application);
    ~LuaEngine();

    void init();
    bool execute(const QString &path, int nr = 0);

    bool pcall(lua_State *L, int na, int nr);
    void printError(int error);

    Document *log()
    { return m_log; }

    void print(const QString &message);

    lua_State *L;

private:
    Document *m_log;
};

#endif // LUAENGINE_H
