#ifndef CONFIGURATIONOBSERVER_H
#define CONFIGURATIONOBSERVER_H

#include <QJsonObject>
#include <QObject>
#include "scopepath.h"

class ConfigurationObserver
{
public:
    ConfigurationObserver();
    virtual ~ConfigurationObserver();

    static ConfigurationObserver *find(QObject *object)
    {
        ConfigurationObserver *s;
        while (object) {
            s = dynamic_cast<ConfigurationObserver*>(object);
            if (s) {
                return s;
            }
            object = object->parent();
        }
        return NULL;
    }

    virtual void configure(QJsonObject &object) = 0;

    virtual void scope(ScopePath *o_path)
    {
        Q_ASSERT(qObject);
        if (!qObject->objectName().isEmpty()) {
            o_path->push(qObject->objectName(), this);
        }
    }

    void enterMode(const QString &_state);
    void leaveMode(const QString &_state);
    void leaveMode();

    virtual ConfigurationObserver *modeBeginEvent(const QString &state) { Q_UNUSED(state); return NULL; }
    virtual void modeEndEvent(const QString &state, ConfigurationObserver *) { Q_UNUSED(state); }

    virtual ConfigurationObserver *proxy() {
        return NULL;
    }

    virtual ConfigurationObserver *scopeParent()
    {
        Q_ASSERT(qObject);
        QObject *o = qObject->parent();
        ConfigurationObserver *s = NULL;
        while (o && s == NULL) {
            s = dynamic_cast<ConfigurationObserver*>(o);
            o = o->parent();
        }
        return s;
    }

    virtual void observeEvent() {}
    virtual void forgetEvent() {}

    QString state;
    QString state_default;
    ConfigurationObserver *state_object;

    QObject *qObject;
};

#endif // CONFIGURATIONOBSERVER_H
