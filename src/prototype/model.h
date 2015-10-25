#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QUuid>
#include "configurationobserver.h"
#include "luaobject.h"

class ScopePath;
class StatusLineController;

class Model : public QObject, public LuaObject, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit Model(QObject *parent = 0);

    QUuid id;

    virtual bool close() { return true; }

    virtual Model *split(QObject *parent)
    { Q_UNUSED(parent); return NULL; }

    virtual StatusLineController *statusLineController()
    { return NULL; }

    virtual void scopePath(ScopePath *) {}

    virtual QString description() = 0;

signals:

public slots:
};

#endif // MODEL_H
