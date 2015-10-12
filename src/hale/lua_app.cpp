#define CLASS_APP "app"

#include <QDir>

#include <lua.hpp>
#include "luaengine.h"

#include "application.h"
#include "model.h"
#include "commandmanager.h"

#include "documentmodel.h"
#include "panel.h"
#include "consolemodel.h"
#include "luautil.h"

// #include "luaconsole.cpp"

#include "lua_consolemodel.h"
#include "lua_documentmodel.h"
#include "lua_scopepath.h"

//
// Configuration
//

static int
luaapp_configure(lua_State *L)
{
    // auto options(tlua::read<tlua::table>(L, 1));
    // Application::instance()->reloadConfiguration();
    return 0;
}

//
// Commands
//

//void luaapp_actionTriggered()
//{
//}

static int
luaapp_addCommand(lua_State *L)
{
    auto cm = Application::instance()->commandManager();
    tlua::table command;
    if (lua_istable(L, 1)) {
        command = tlua::read<tlua::table>(L, 1);
        QString name(command.get<QString>("name"));
        QString scope(command.get<QString>("scope"));
        cm->addCommand(name, scope, command);
    } else {
        QString name = tlua::read<QString>(L, 1);
        QString scope = tlua::read<QString>(L, 2);

        if (lua_isfunction(L, 3)) {
            command = tlua::table::create(L, 0, 3);
            command.setv<tlua::function>("execute", tlua::function(L, 3));
        } else if (lua_istable(L, 3)) {
            command = tlua::table(L, 3);
        } else {
            luaL_error(L, LERROR_PARAM_N_EXPECTED_TYPE(3, "function or table"));
        }

        command.set<QString>("name", name);
        command.set<QString>("scope", scope);
        cm->addCommand(QString(name), QString(scope), command);
    }

    return command.push(L);
}

static int
luaapp_findCommand(lua_State *L)
{
    auto cm = Application::instance()->commandManager();
    QString name(tlua::read<QString>(L, 1));
    auto command = cm->findCommand(name);
    if (!command) {
        lua_pushnil(L);
        return 1;
    }
    return command->lua_command.push(L);
}

static int
luaapp_addKey(lua_State *L)
{
    QString name(tlua::read<QString>(L, 1));
    QString key(tlua::read<QString>(L, 2));

    auto app = Application::instance();
    app->commandManager()->addKeyDown(name, key);

    return 0;
}

static int
luaapp_addKeyUp(lua_State *L)
{
    QString name(tlua::read<QString>(L, 1));
    QString key(tlua::read<QString>(L, 2));

    auto app = Application::instance();
    app->commandManager()->addKeyUp(name, key);

    return 0;
}

static int
luaapp_execute(lua_State *L)
{
    // name
    QString name(tlua::read<QString>(L, 1));
    auto app = Application::instance();
    auto command = app->commandManager()->findCommand(name);
    if (command) {
        app->commandManager()->execute(command, app->scopePath());
    } else {
        luaL_error(L, LERROR_F_NOT_FOUND, name.constData());
    }
    return 0;
}

static int
luaapp_startIdleTimer(lua_State *L)
{
    Application::instance()->idle_timer->start(0);
    return 0;
}

static int
luaapp_stopIdleTimer(lua_State *L)
{
    Application::instance()->idle_timer->stop();
    return 0;
}

//
// Configuration
//

static int
luaapp_updateFile(lua_State *L)
{
    QString path(tlua::read<QString>(L, 1));
    Application::instance()->updateFile(path);
    return 0;
}

//
// Documents
//

static int
luaapp_openDocument(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    Application *app = Application::instance();
    app->setActiveModel(document);

    return 0;
}

static int
luaapp_splitDocument(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    auto app = Application::instance();
    int direction = luaL_checkinteger(L, 2);

    auto model = qobject_cast<DocumentModel*>(app->splitModel(document, (Direction)direction));

    return tlua::push<DocumentModel*>(L, model);
}

static int
luaapp_saveDocument(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    lua_pushboolean(L, Application::instance()->saveDocumentModel(document));

    return 1;
}

//
// Panels
//

static int
luaapp_requirePanel(lua_State *L)
{
    return tlua::pushv<Panel*>(L, Application::instance()->requirePanel());
}

//
// Utility
//

static int
luaapp_resourcePath(lua_State *L)
{
    QByteArray path(Application::instance()->resourcePath().toUtf8());
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addlstring(&b, path.constData(), path.length());
//    char *b_ptr = luaL_buffinitsize(L, &b, path.length());
//    memcpy(b_ptr, path.constData(), path.length());
//    // printf("%x allocated %d\n", (size_t)b_ptr, off_e-off_b);
//    doc_text(&ldoc->doc, off_b, off_e - off_b, b_ptr);
    luaL_pushresult(&b);
    return 1;
}

static int
luaapp_changeDirectory(lua_State *L)
{
    QString path(tlua::read<QString>(L, 1));
    bool ret = QDir::setCurrent(path);
    return tlua::push<bool>(L, ret);
}

static int
luaapp_print(lua_State *L)
{
    QString message;

    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
      const char *s;
      lua_pushvalue(L, -1);  /* function to be called */
      lua_pushvalue(L, i);   /* value to print */
      lua_call(L, 1, 1);
      s = lua_tostring(L, -1);  /* get result */
      if (s == NULL)
        return luaL_error(L, "tostring must return a string to print");
      if (i>1) {
          // fputs("\t", stdout);
          message.append("\t");
       }
      // fputs(s, stdout);
      message.append(s);
      lua_pop(L, 1);  /* pop result */
    }

    // message.append("\n");
    Application::instance()->luaEngine()->print(message);
    // fputs("\n", stdout);
    // d << "\n";
    return 0;
}

static int
luaapp_setController(lua_State *L)
{
    Application::instance()->lua_controller = tlua::table(L, 1);
    return 0;
}

static int
luaapp_console(lua_State *L)
{
    return tlua::pushv<ConsoleModel*>(L, Application::instance()->console());
}

static int
luaapp_enterMode(lua_State *L)
{
    auto app = Application::instance();
    auto mode = tlua::read<QString>(L, 1);
    if (mode.length() == 0) {
        luaL_error(L, LERROR_PARAM_N_EXPECTED(1));
    }
    app->enterMode(mode);

    return 0;
}

static int
luaapp_leaveMode(lua_State *L)
{
    auto app = Application::instance();
    if (lua_gettop(L) == 0) {
        app->leaveMode();
    } else {
        auto mode = tlua::read<QString>(L, 1);
        app->leaveMode(mode);
    }

    return 0;
}


static int
luaapp_beginSwitch(lua_State *)
{
//    auto app = Application::instance();
//    app->beginSwitch(app->activePanel());

    return 0;
}

static int
luaapp_endSwitch(lua_State *)
{
//    auto app = Application::instance();
//    app->endSwitch();

    return 0;
}

static int
luaapp_switch(lua_State *L)
{
    auto app = Application::instance();
    int command = tlua::read<int>(L, 1);
    app->switchCommand((SwitchCommand)command);
    return 0;
}

int lua_app(lua_State *L)
{
    static const struct luaL_Reg
    functions[] = {
        {"configure", luaapp_configure},

        {"addCommand", luaapp_addCommand},
        {"findCommand", luaapp_findCommand},
        {"addKey", luaapp_addKey},
        {"addKeyUp", luaapp_addKeyUp},
        {"execute", luaapp_execute},

        {"startIdleTimer", luaapp_startIdleTimer},
        {"stopIdleTimer", luaapp_stopIdleTimer},

        {"updateFile", luaapp_updateFile},

        // {"document", luaapp_document},

        {"requirePanel", luaapp_requirePanel},
        {"saveDocument", luaapp_saveDocument},
        {"openDocument", luaapp_openDocument},
        {"splitDocument", luaapp_splitDocument},
        {"controller", luaapp_setController},

        {"enterMode", luaapp_enterMode},
        {"leaveMode", luaapp_leaveMode},
        {"beginSwitch", luaapp_beginSwitch},
        {"endSwitch", luaapp_endSwitch},
        {"switch", luaapp_switch},

        {"resourcePath", luaapp_resourcePath},
        {"changeDirectory", luaapp_changeDirectory},
        {"print", luaapp_print},

        {"console", luaapp_console},
        {NULL, NULL}
    };


    tlua::table app(tlua::table::create(L, 0, 0));

    app.set(functions);

    app.setv("ORIENTATION_HORIZONTAL", (int)Qt::Horizontal);
    app.setv("ORIENTATION_VERTICAL", (int)Qt::Vertical);

    app.setv("DIRECTION_LEFT", (int)Direction::Left);
    app.setv("DIRECTION_RIGHT", (int)Direction::Right);
    app.setv("DIRECTION_UP", (int)Direction::Up);
    app.setv("DIRECTION_DOWN", (int)Direction::Down);

    app.setv("NEXT_MODEL", (int)SwitchCommand::NextModel);
    app.setv("PREVIOUS_MODEL", (int)SwitchCommand::PreviousModel);
    app.setv("NEXT_PANEL", (int)SwitchCommand::NextPanel);
    app.setv("PREVIOUS_PANEL", (int)SwitchCommand::PreviousPanel);

//    app.push(L);
//    luautil_wrap(L, Application::instance()->console());
//    lua_setfield(L, -2, "console");
//    lua_pop(L, 1);



//    lua_pop(L, 1);

    return app.push(L);
}
