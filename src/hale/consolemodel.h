#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include "model.h"
#include "scopepath.h"

class LuaEngine;
class DocumentModel;
class Document;
class DocumentEditEvent;
class DocumentRange;

class ConsoleModel : public Model
{
    Q_OBJECT
public:
    ConsoleModel(QObject *parent, LuaEngine *lua);

    int lua_wrap(lua_State *L);

    void scope(ScopePath *o_path);
    void configure(QJsonObject &object);
    void observeEvent();
    void forgetEvent();

//    void setContextScope(const ScopePath &path);

    QString description() { return "Console"; }

    // virtual void scopePath(ScopePath *o_path);

    DocumentModel *logView()
    { return m_log_view; }
    DocumentModel *commandView()
    { return m_command_view; }

    // Sets command to command line
    void complete(tlua::table command);
    // Sets text to command line
    void set(const QString &text);
    // Executes current command on command-line.
    void execute();

    tlua::table lua_controller;
    tlua::userdata lua_context_scope;

private slots:
    void commandEditEnd(DocumentModel *view, bool undo);
    void commandTextChanged(const DocumentEditEvent *event);
    void commandCursorChanged(const DocumentRange &o, const DocumentRange &n);

    void historyChanged();

private:
    ScopePath m_context_scope;
    DocumentModel *m_log_view;
    Document *m_command_document;
    DocumentModel *m_command_view;

    void emitUpdate(bool text, bool cursor);

    void updateContext();
};

LUAOBJECT_DECLARE(ConsoleModel)

#endif // CONSOLEMODEL_H
