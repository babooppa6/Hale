#ifndef COMPLETION_H
#define COMPLETION_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <QTextLayout>
#include <QAbstractProxyModel>
#include <QItemSelectionModel>

#include "documenttypes.h"

class CompletionSelection;
class CompletionItemDelegate;

class QAbstractItemModel;
class QItemSelectionModel;

class CompletionController : public QObject
{
    Q_OBJECT
public:
    explicit CompletionController(DocumentModel *view, QObject *parent = 0);

    void begin(const DocumentPosition &begin, const DocumentPosition &end, const DocumentPosition &cursor);
    void end();
    void update(const DocumentPosition &begin, const DocumentPosition &end, int shift);

    void next();
    void previous();
    void confirm();

    const DocumentPosition &beginPosition()
    { return m_begin; }
    const DocumentPosition &endPosition()
    { return m_end; }
    const QString &text()
    { return m_text; }

    void complete(int begin, int end, const QString &text);

    DocumentModel *documentView();
    Document *document();

    virtual int replacementBegin() = 0;
    virtual int replacementEnd() = 0;
    virtual QAbstractItemModel *model() = 0;
    virtual QItemSelectionModel *selectionModel() = 0;

protected:
    virtual void beginEvent() {}
    virtual void endEvent() {}
    virtual void completeEvent(const QString &text, const DocumentPosition &begin, const DocumentPosition &end, int cursor) {
        Q_UNUSED(text);
        Q_UNUSED(begin);
        Q_UNUSED(end);
        Q_UNUSED(cursor);
    }
    virtual void confirmEvent() {}

signals:
    void begun();
    void updated();
    void ended();

public slots:

private:
    DocumentModel *m_dv;
    DocumentPosition m_begin;
    DocumentPosition m_end;
    int m_cursor;
    QString m_text;

    void update();
};


class CompletionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CompletionWindow(QWidget *parent = 0);

    void setCompletionRect(const QRect &rect);
    void setModel(QAbstractItemModel *model, QItemSelectionModel *selection);
    void setFilter(const QString &filter);

    QAbstractItemModel *model;
    QItemSelectionModel *selection;

protected:
    bool event(QEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
//    void focusInEvent(QFocusEvent *);
//    void focusOutEvent(QFocusEvent *);

signals:

private slots:
//    void rowsRemoved(const QModelIndex & parent, int first, int last);
//    void rowsInserted(const QModelIndex & parent, int first, int last);
    void modelReset();

private:
    QRectF m_completion_rect;
    CompletionItemDelegate *m_item_delegate;
    QAbstractItemModel *m_model;
    QListView *m_list;
};

class CompletionItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    CompletionItemDelegate(QObject *parent = 0);

    void setFilter(const QString &filter);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QString m_filter;
    QTextLayout m_text_layout;
};

class CompletionSelection : public QItemSelectionModel
{
    Q_OBJECT
public:
    CompletionSelection(QAbstractItemModel *model, QObject *parent);

    bool selectNextIndex();
    bool selectPreviousIndex();
};
#endif // COMPLETION_H
