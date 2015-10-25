#ifndef PATHCOMPLETIONCONTROLLER_H
#define PATHCOMPLETIONCONTROLLER_H

#include <QSharedPointer>
#include <QStringListModel>
#include "completion.h"

class PathIndexFilter;

class PathCompletionController : public CompletionController
{
public:
    PathCompletionController(DocumentModel *view, QObject *parent);

    virtual QAbstractItemModel *model()
    { return m_model; }
    virtual QItemSelectionModel *selectionModel()
    { return m_selection; }
    virtual int replacementBegin()
    { return m_replacement_begin; }
    virtual int replacementEnd()
    { return m_replacement_end; }

private slots:
    void results(const QStringList &results);

protected:
    virtual void completeEvent(const QString &text, const DocumentPosition &begin, const DocumentPosition &end, int cursor);
    // virtual void confirmEvent();

private:
    QStringListModel *m_model;
    QItemSelectionModel *m_selection;
    QSharedPointer<PathIndexFilter> m_filter;
    int m_replacement_begin;
    int m_replacement_end;
};

#endif // PATHCOMPLETIONCONTROLLER_H
