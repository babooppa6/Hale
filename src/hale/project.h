#ifndef PROJECT_H
#define PROJECT_H

#include "luaobject.h"
#include "tlua.h"

#include <QObject>
#include <QVector>

class Document;

class Project : public QObject, public LuaObject
{
    Q_OBJECT
public:
    static Project *createProject(const QString &path, QObject *parent);

    Project(const QString &path, QObject *parent);

    QString path;
    tlua::table lua_controller;

    void documentCreated(Document *document);
    void documentLoaded(Document *document);
    void documentSaved(Document *document);

public slots:

private:
    void call(const char *f, Document *document);
};

LUAOBJECT_DECLARE(Project)

#endif // PROJECT_H
