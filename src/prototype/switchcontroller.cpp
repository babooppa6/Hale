#include <QStringListModel>
#include <QJsonValue>
#include "application.h"
#include "option.h"
#include "panel.h"
#include "model.h"
#include "completion.h"
#include "switchcontroller.h"

SwitchController::SwitchController(Application *application) :
    QObject(application),
    m_window(NULL),
    m_application(application)
{
    qObject = this;
}

bool SwitchController::isActive()
{
    return m_window && m_window->isVisible();
}

void SwitchController::scope(ScopePath *o_path)
{
}

void SwitchController::configure(QJsonObject &object)
{
    set(&options, object);
}

void SwitchController::show(Panel *panel)
{
    Q_ASSERT(panel);
    m_panel = panel;

    if (m_window == NULL) {
        m_window = new CompletionWindow(panel->window());
        auto m = new QStringListModel(m_window);
        auto s = new CompletionSelection(m, m_window);
        connect(s, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChanged(QModelIndex,QModelIndex)));
        m_window->setModel(m, s);
    }

    m_models.clear();
    QStringListModel *model = qobject_cast<QStringListModel*>(m_window->model);
    QStringList data;
    Panel::Models::iterator it = panel->models.end();
    while (it != panel->models.begin()) {
        it--;
        m_models.append(*it);
        data.append((*it)->description());
    }
    model->setStringList(data);

    set(m_window, &options);
    m_window->show();
}

void SwitchController::hide()
{
    m_window->hide();
    // TODO: This has to be in "confirm"
    if (m_panel) {
        m_application->focusPanel(m_panel);
        m_panel->setModel(m_panel->current_model);
    }
}

void SwitchController::confirm()
{
    auto selection = qobject_cast<CompletionSelection*>(m_window->selection);
    auto index = selection->currentIndex();
}

void SwitchController::command(SwitchCommand command)
{
    auto selection = qobject_cast<CompletionSelection*>(m_window->selection);
    switch (command)
    {
    case SwitchCommand::NextModel:
        selection->selectNextIndex();
        break;
    case SwitchCommand::PreviousModel:
        selection->selectPreviousIndex();
        break;
    case SwitchCommand::NextPanel: {
        auto panel = m_application->nextPanel(m_panel);
        if (panel != NULL) {
            show(panel);
        }
    } break;
    case SwitchCommand::PreviousPanel: {
        auto panel = m_application->previousPanel(m_panel);
        if (panel != NULL) {
            show(panel);
        }
    } break;
    default:
        Q_ASSERT(0 && "Not reached.");
        break;
    }

}

void SwitchController::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    auto model = m_models[current.row()];
    m_panel->peekModel(model);
}
