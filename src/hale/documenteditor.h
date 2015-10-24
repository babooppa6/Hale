#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>

#include "enums.h"
#include "configurationobserver.h"
#include "documentmodel.h"

class DocumentModel;
class DocumentEditorLayout;
class StatusLine;
class CompletionWindow;

class DocumentEditor : public QWidget, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit DocumentEditor(QWidget *parent = 0);
    ~DocumentEditor();

    void configure(QJsonObject &object);
    void scope(ScopePath *o_path);
    ConfigurationObserver *proxy();

    enum struct ScrollPolicy
    {
        /// Scroll to primary caret. Default scrolling behavior in editor.
        Default = 0,
        /// Scroll so the last block is fully visible on the bottom.
        /// This is done automatically when:
        /// - Editor is resized
        /// - Text was changed
        /// Editor does the last line scroll only in case user is looking at
        /// the last line right now. This still allows users to use caret and selections
        /// in the text above.
        Last
    };

    enum struct ContentSizePolicy
    {
        Default = 0,
        Minimal
    };

    void setContentSizePolicy(ContentSizePolicy policy);
    void setScrollPolicy(ScrollPolicy policy);

    /// Moves view from one editor to other editor.
    static void moveDocumentView(DocumentModel *view, DocumentEditor* editor);
    void addDocumentView(DocumentModel *view);
    void removeDocumentView(DocumentModel *view);
    void setActiveDocumentView(DocumentModel *view);

    DocumentModel *activeDocumentView() const
    { return m_dv; }

    /// Returns current viewport rectangle.
    virtual QRectF viewport();
    /// Scrolls to given position.
    virtual void scrollTo(qreal x, qreal y);
    /// Scrolls by given deltas.
    virtual void scrollBy(qreal dx, qreal dy);
    /// Scrolls so that range is optimaly visible with emphasis on r.first.
    virtual void scrollToRange(const DocumentRange &r);
    /// Scrolls so that position is optimally visible.
    void scrollToPosition(const DocumentPosition &p);

    //
    // Widget layouting
    //

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    //
    //
    //

    /// Gutter width that is calculated by the editor.
    qreal normalGutterWidth();
    /// Gutter width that is set externally to the editor. If none is set, then returns normalGutterWidth()
    qreal gutterWidth();
    /// Sets gutter width, but only in case the gutter_width is larger than normalGutterWidth()
    void setGutterWidth(qreal gutter_width);

protected:
    virtual void moveEvent(QMoveEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

signals:
    void normalGutterWidthChanged();
    void focused();
    void documentViewChanged(DocumentModel *old_view, DocumentModel *new_view);

private slots:
    void configured(const DocumentOptions &previous);
    void beforeTextChanged();
    void textChanged(const DocumentEditEvent *event);
    void beforeFormatChanged();
    void formatChanged(int block);
    void cursorChanged(const DocumentRange &o, const DocumentRange &n);
    void themeChanged(QSharedPointer<Theme> theme);

private:
    typedef QList<DocumentModel *> DocumentViews;
    DocumentViews m_views;
    DocumentModel *m_dv;
    DocumentEditorLayout *m_layout;
    ScrollPolicy m_scroll_policy;
    ContentSizePolicy m_content_size_policy;

    bool m_was_last_block_visible;
    qreal m_height_preferred;
    void updatePreferredHeight();

    bool isLastBlockVisible();
    void scrollToLastBlock();

public:
    //
    // Completion
    //

private slots:
    void completionBegin();
    void completionUpdated();
    void completionEnd();

private:
    CompletionWindow *m_completion_window;

    void updateCompletionWindowPosition();
};

//
//
//

class DocumentEditorGroup : public QObject
{
    Q_OBJECT
public:
    DocumentEditorGroup(QObject *parent);

    void addEditor(DocumentEditor *editor);

private slots:
    void updateGutterWidths();

private:
    typedef QList<DocumentEditor *> Editors;
    Editors m_editors;
};


#endif // EDITOR_H
