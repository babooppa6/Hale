#include "configuration.h"
#include "configurationobserver.h"

ConfigurationObserver::ConfigurationObserver() :
    qObject(NULL),
    state_object(NULL)
{}

ConfigurationObserver::~ConfigurationObserver()
{
    Configuration::instance->forget(this);
}

void ConfigurationObserver::enterMode(const QString &_state)
{
    if (state != _state) {
        state = _state;
        state_object = modeBeginEvent(state);
        Configuration::instance->update(this);
    }
}

void ConfigurationObserver::leaveMode(const QString &_state)
{
    Q_ASSERT(_state == state && _state != state_default);
    leaveMode();
}

void ConfigurationObserver::leaveMode()
{
    if (state != state_default) {
        QString previous(state);
        auto previous_object = state_object;
        state = "";
        state_object = NULL;
        Configuration::instance->update(this);

        modeEndEvent(previous, previous_object);

        enterMode(state_default);
    }
}
