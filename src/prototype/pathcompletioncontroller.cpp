#include <QStringListModel>
#include <QItemSelectionModel>

#include "application.h"
#include "pathcompleter.h"
#include "pathcompletioncontroller.h"

PathCompletionController::PathCompletionController(DocumentModel *view, QObject *parent) :
    CompletionController(view, parent)
{
    m_model = new QStringListModel(this);
    m_selection = new QItemSelectionModel(m_model, this);
}

void PathCompletionController::completeEvent(const QString &text, const DocumentPosition &begin, const DocumentPosition &end, int cursor)
{
    Q_UNUSED(begin);
    Q_UNUSED(end);

    m_model->setStringList(QStringList());

    auto path_completer = Application::instance()->pathCompleter();
    if (m_filter) {
        disconnect(m_filter.data(), 0, this, 0);
    }

    m_filter = path_completer->completePath(text, cursor, FilterType::Single);

    if (m_filter) {
        connect(m_filter.data(), &PathIndexFilter::resultsAvailable, this, &PathCompletionController::results);
        m_replacement_begin = m_filter->replacement_begin;
        m_replacement_end = m_filter->replacement_end;
    }
}

void PathCompletionController::results(const QStringList &results)
{
    if (sender() != m_filter.data()) {
        qDebug() << __FUNCTION__ << "------------------------- OUTDATED RESULTS";
        return;
    };

    qDebug() << __FUNCTION__ << "received" << results.size() << "results";

    m_model->setStringList(results);
}
