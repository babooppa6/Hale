#include "precompiled.h"
#include "document.h"
#include "luaengine.h"
#include "project.h"

Project *Project::createProject(const QString &path, QObject *parent)
{
    QDir dir(path);
    if (!dir.exists(".hale")) {
        qWarning() << __FUNCTION__ << ".hale file for project was not found in" << path;
        return NULL;
    }

    if (!LuaEngine::instance->execute(dir.absoluteFilePath(".hale"), 1)) {
        qWarning() << __FUNCTION__ << ".hale file contains errors";
        return NULL;
    }

    // tlua::dump(LuaEngine::instance->L);
    tlua::table controller(tlua::pop<tlua::table>(LuaEngine::instance->L));
    if (controller.empty()) {
        qWarning() << __FUNCTION__ << ".hale file does not return a table.";
        return NULL;
    }

    auto project = new Project(path, parent);
    project->lua_controller = controller;
    return project;
}

Project::Project(const QString &path, QObject *parent) :
    QObject(parent),
    path(path)
{
}

void Project::documentSaved(Document *document)
{
    call("documentSaved", document);
}

void Project::documentLoaded(Document *document)
{
    call("documentLoaded", document);
}

void Project::documentCreated(Document *document)
{
    call("documentCreated", document);
}

void Project::call(const char *function_name, Document *document)
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>(function_name));
    if (f.empty()) {
        return;
    }

    // self
    tlua::push(f.L, lua_controller);
    // project
    tlua::push(f.L, lua_self);
    // document
    tlua::push<Document*>(f.L, document);

    f.invoke(3, 0);
}
