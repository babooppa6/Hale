#include "precompiled.h"

#include "tlua.h"
#include "lua_scopepath.h"
#include "configuration.h"
#include "panel.h"

#include "application.h"
#include "document.h"
#include "documentmodel.h"
#include "luaengine.h"
#include "consolemodel.h"

ConsoleModel::ConsoleModel(QObject *parent, LuaEngine *lua) :
    Model(parent)
{
//    GutterOptions options;
//    options.padding.setLeft(16);
//    options.padding.setRight(4);
//    options.type = GutterOptions::GutterType::Circle;
    // m_log_document = new Document(this);
    m_log_view = new DocumentModel(lua->log(), this);
    m_log_view->setObjectName("document.log");

    // m_log_view->setGutterOptions(options);

    // NOTE: Do not set the parent here, as the model is owned already,
    // and model will trigger deletion of the document.
    m_command_document = new Document();
    m_command_view = new DocumentModel(m_command_document, this);
    m_command_view->setObjectName("document.command");
    // options.type = GutterOptions::GutterType::ArrowRight;
    // m_command_view->setGutterOptions(options);
    connect(m_command_view, &DocumentModel::editEnd, this, &ConsoleModel::commandEditEnd);
    connect(m_command_view, &DocumentModel::textChanged, this, &ConsoleModel::commandTextChanged);
    connect(m_command_view, &DocumentModel::cursorChanged, this, &ConsoleModel::commandCursorChanged);

    auto grammar = GrammarManager::instance->findGrammar("source.lua");
    if (grammar) {
        m_command_document->setGrammar(grammar);
    }

    connect(Application::instance(), SIGNAL(historyChanged()), this, SLOT(historyChanged()));
    updateContext();

    lua_context_scope = lua_scopepath_wrap(lua->L, &m_context_scope);
}

void ConsoleModel::historyChanged()
{
    updateContext();
}

void ConsoleModel::updateContext()
{
    auto application = Application::instance();
    auto panel = application->activeNonConsolePanel();
    if (panel == NULL) {
        m_context_scope.clear();
    } else {
        if (panel->model()) {
            m_context_scope.make(panel->model());
        } else {
            m_context_scope.make(panel);
        }
        // application->makeScopePath(&m_context_scope, panel, NULL);
        qDebug() << __FUNCTION__ << "Console context scope changed to";
        m_context_scope.dump();
    }
}

void ConsoleModel::configure(QJsonObject &object)
{
}

void ConsoleModel::scope(ScopePath *o_path)
{
    o_path->push("console", this);
}

void ConsoleModel::observeEvent()
{
    Configuration::instance->observe(m_command_view);
    Configuration::instance->observe(m_log_view);
}

void ConsoleModel::forgetEvent()
{
    Configuration::instance->forget(m_command_view);
    Configuration::instance->forget(m_log_view);
}

//void ConsoleModel::setContextScope(const ScopePath &path)
//{
//    qDebug() << __FUNCTION__ << "Scope";
//    m_context_scope.dump();
//    m_context_scope = path;
//}

void ConsoleModel::complete(tlua::table command)
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>("complete"));
    if (f.empty()) {
        return;
    }

    // self
    tlua::push(f.L, lua_controller);
    // console
    tlua::push(f.L, lua_self);
    // command
    tlua::push(f.L, command);

    f.invoke(3, 0);
}

void ConsoleModel::set(const QString &text)
{
    auto e = m_command_view->edit();
    m_command_view->clear(e);
    m_command_view->insertText(e, text);
}

void ConsoleModel::execute()
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>("execute"));
    if (f.empty()) {
        return;
    }

    QString t(m_command_view->text());

    // self
    tlua::push(f.L, lua_controller);
    // console
    tlua::push(f.L, lua_self);
    // text
    tlua::push(f.L, t);

    f.invoke(3, 0);

    auto e = m_command_view->edit();
    m_command_view->clear(e);
}

void ConsoleModel::commandEditEnd(DocumentModel *, bool)
{
    emitUpdate(true, false);
}

void ConsoleModel::commandTextChanged(const DocumentEditEvent *)
{
    // Replaced by commandEditEnd
}

void ConsoleModel::commandCursorChanged(const DocumentRange &, const DocumentRange &)
{
    if (!m_command_view->isEditing()) {
        emitUpdate(false, true);
    }
}

void ConsoleModel::emitUpdate(bool text, bool cursor)
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>("update"));
    if (f.empty()) {
        return;
    }

    // self
    tlua::push(f.L, lua_controller);
    // console
    tlua::push(f.L, lua_self);
    // text
    tlua::push(f.L, text);
    // cursor
    tlua::push(f.L, cursor);

    f.invoke(4, 0);

    luautil_stack_dump(f.L);
}
