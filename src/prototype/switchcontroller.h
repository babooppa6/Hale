#ifndef SWITCHCONTROLLER_H
#define SWITCHCONTROLLER_H

#include <QObject>
#include "configurationobserver.h"
#include "option.h"
#include "enums.h"

class Model;
class Panel;
class Application;
class CompletionWindow;

class SwitchController : public QObject, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit SwitchController(Application *application);

    CommonOption options;

    void scope(ScopePath *o_path);
    void configure(QJsonObject &object);

    bool isActive();

    void show(Panel *panel);
    void hide();

    void command(SwitchCommand command);

    void confirm();

signals:

private slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    QList<Model *> m_models;
    Application *m_application;
    Panel *m_panel;
    CompletionWindow *m_window;
};

#endif // SWITCHCONTROLLER_H
