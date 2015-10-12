#include <QDebug>
#include "lua_scopepath.h"
#include "scopepath.h"
#include "commandmanager.h"

namespace
{
//    void luacommand_function(Command *command, ScopePath *scope)
//    {
//        luascopepath_push(command->lua_func.L, scope);
//        command->lua_func.invoke(1, 0);
//    }
}

CommandManager::CommandManager(QObject *parent) :
    QObject(parent)
{
}

void CommandManager::addCommand(const QString &id, const QString &scope, CommandCall)
{
    Command command;
    command.id = id;
    command.selector = scope;
    // command.execute = f;
    command.action = NULL;// new CommandAction(command.id);

    m_commands.append(command);
    Command *command_ptr = &m_commands.last();
    m_command_map.insert(id, command_ptr);
}

void CommandManager::addCommand(const QString &id, const QString &scope, tlua::reference lua_command)
{
    Command command;
    command.id = id;
    command.selector = ScopeSelector(scope);
    // command.execute = &luacommand_function;
    command.lua_command = lua_command;
    command.action = NULL; // new CommandAction(command.id);

    m_commands.append(command);
    Command *command_ptr = &m_commands.last();
    m_command_map.insert(id, command_ptr);
}

void CommandManager::addKeyDown(const QString &command_id, const QString &key)
{
    addKey(&m_key_down_map, command_id, key);
}

void CommandManager::addKeyUp(const QString &command_id, const QString &key)
{
    addKey(&m_key_up_map, command_id, key);
}

void CommandManager::addKey(KeyMap *map, const QString &command_id, const QString &key)
{
    Command *command = findCommand(command_id);
    if (command == NULL) {
        // TODO(cohen): Store the keys in a temporary string->key map
        // and assign them when the command is available.
        qWarning() << "Command" << command_id << "not found.";
        Q_ASSERT(0 && "Command not found.");
        return;
    }

    // TODO(cohen) QKeySequence::PortableText?
    QKeySequence key_sequence;
    if (key == "Ctrl") {
        key_sequence = QKeySequence(Qt::Key_Control);
    } else {
        key_sequence = QKeySequence(key);
        if (key_sequence.isEmpty()) {
            qWarning() << "Key" << key << "for command" << command_id << "was invalid.";
        }
    }

    // qDebug() << __FUNCTION__ << key_sequence;
    // TODO(cohen) Support sequences?
    map->insert(key_sequence[0], command);
}

bool CommandManager::handleKeyEvent(QObject *obj, QKeyEvent *event, ScopePath *scope_path)
{
    Q_UNUSED(obj);

    KeyMap *key_map = &m_key_down_map;
    if (event->type() == QEvent::KeyRelease) {
        key_map = &m_key_up_map;
        // qDebug() << __FUNCTION__ << "KeyRelease" << event->key();
    } else {
        // qDebug() << __FUNCTION__ << "KeyPress" << event->key();
    }

    // TODO(cohen): In some cases I will have to wait for the scope.
    // - In that case, any request for handling a key event will have to buffer the events.
    // - Once the scope is available (could be few frames later) we will replay the buffer.
    // - So if the editor will get a request for scope, a lambda will be requested.
    // - Editor will wait until the scope is ready at given caret location and call the lambda.


    int key = event->modifiers() | event->key();

    // qDebug() << QKeySequence(key).toString(QKeySequence::PortableText) << QKeySequence(key).toString(QKeySequence::NativeText);
    KeyMap::iterator it = key_map->lowerBound(key);
    if (it == key_map->end()) {
        return false;
    }

    QList<Command*> candidates;
    ScopeScore top;
    top.reset();
    ScopeScore current;
    int c;
    while (it.key() == key) {
        if (scope_path->match(it.value()->selector, &current)) {
            c = current.compare(top);
            // qDebug() << __FUNCTION__ << it.value()->id << "current" << current << "top" << top << "c" << c;
            if (c >= 0) {
                if (c != 0) {
                    candidates.clear();
                    top = current;
                }
                candidates.append(it.value());
            }
        } else {
            // qDebug() << __FUNCTION__ << it.value()->id << "NO MATCH" << it.value()->selector.string_list;
        }
        it++;
    }

    if (!candidates.empty()) {
        emit executeCommands(candidates, scope_path);
        return true;
    }
    return false;
}

void CommandManager::execute(Command *command, const ScopePath *scope_path)
{
    QList<Command *> commands;
    commands.append(command);
    emit executeCommands(commands, scope_path);
}

Command *CommandManager::findCommand(const QString &id)
{
    CommandMap::iterator it = m_command_map.find(id);
    if (it == m_command_map.end()) {
        return NULL;
    }
    return it.value();
}
