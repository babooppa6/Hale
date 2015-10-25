#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>
#include <QVector>
#include <QKeyEvent>

#include <functional>
#include "tlua.h"

#include "scopepath.h"

class CommandSpec;
class CommandContext;
class ScopePath;

struct Command;
typedef std::function<void (Command *, ScopePath *)> CommandCall;

struct Command
{
    QString id;
    ScopeSelector selector;
    CommandCall execute;
    QAction *action;
    tlua::reference lua_command;
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    explicit CommandManager(QObject *parent = 0);

    void addCommand(const QString &id, const QString &scope, CommandCall f);
    void addCommand(const QString &id, const QString &scope, tlua::reference lua_command);
    void addKeyDown(const QString &command_id, const QString &key);
    void addKeyUp(const QString &command_id, const QString &key);

    Command *findCommand(const QString &id);

    bool handleKeyEvent(QObject *obj, QKeyEvent *event, ScopePath *scope_path);

    void execute(Command *command, const ScopePath *scope_path);

signals:
    void executeCommands(const QList<Command*> &commands, const ScopePath *scope_path);

public slots:

private:

    typedef QList<Command> Commands;
    Commands m_commands;

    typedef QList<Command*> CommandPtrs;

    typedef QHash<QString, Command*> CommandMap;
    CommandMap m_command_map;

    typedef QMultiMap<int, Command*> KeyMap;
    KeyMap m_key_down_map;
    KeyMap m_key_up_map;

    void addKey(KeyMap *map, const QString &command_id, const QString &key);

    // void execute(Command *command);
};


#endif // COMMANDMANAGER_H
