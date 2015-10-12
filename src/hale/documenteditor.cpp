#include "precompiled.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

#include "configuration.h"
#include "util.h"

#include "document.h"
#include "documentmodel.h"
#include "theme.h"

#include "completion.h"

// #include "statusline.h"
#include "documenteditorlayout.h"
#include "documenteditor.h"

DocumentEditor::DocumentEditor(QWidget *parent) :
    QWidget(parent),
    m_dv(NULL),
    m_layout(NULL),
    m_height_preferred(0),
    m_completion_window(NULL),
    m_scroll_policy(ScrollPolicy::Default),
    m_content_size_policy(ContentSizePolicy::Default)
{
    qObject = this;
    connect(Configuration::instance, &Configuration::themeChanged, this, &DocumentEditor::themeChanged);
    setFocusPolicy(Qt::StrongFocus);
}

DocumentEditor::~DocumentEditor()
{

}

void DocumentEditor::configure(QJsonObject &object)
{
    Q_ASSERT(false && "Not called");
}

void DocumentEditor::scope(ScopePath *o_path)
{
    Q_ASSERT(false && "Not called");
}

ConfigurationObserver *DocumentEditor::proxy()
{
    return m_dv;
}

void DocumentEditor::setContentSizePolicy(ContentSizePolicy policy)
{
    if (policy != m_content_size_policy) {
        m_content_size_policy = policy;
        QSizePolicy size_policy(sizePolicy());
        switch (m_content_size_policy) {
        case ContentSizePolicy::Default:
            size_policy.setVerticalPolicy(QSizePolicy::MinimumExpanding);
            break;
        case ContentSizePolicy::Minimal:
            size_policy.setVerticalPolicy(QSizePolicy::Fixed);
            break;
        }
        setSizePolicy(size_policy);

        updateGeometry();
    }
}

void DocumentEditor::setScrollPolicy(ScrollPolicy policy)
{
    if (m_scroll_policy != policy) {
        m_scroll_policy = policy;
        scrollToLastBlock();
    }
}

QSize DocumentEditor::sizeHint() const
{
    return QSize(160, m_dv->lineHeight() + m_dv->options.padding.top() + m_dv->options.padding.bottom());
//    switch (m_content_size_policy) {
//    case ContentSizePolicy::Minimal:

//    }

//    return size;
}

QSize DocumentEditor::minimumSizeHint() const
{
    return QSize(160, m_dv->lineHeight() + m_dv->options.padding.top() + m_dv->options.padding.bottom());
}

qreal DocumentEditor::normalGutterWidth()
{
    if (!m_layout) {
        return 0;
    }
    return m_layout->normalGutterWidth();
}

qreal DocumentEditor::gutterWidth()
{
    if (!m_layout) {
        return 0;
    }
    return m_layout->gutterWidth();
}

void DocumentEditor::setGutterWidth(qreal gutter_width)
{
    if (!m_layout) {
        return;
    }
    m_layout->setGutterWidth(gutter_width);
}

void DocumentEditor::themeChanged(QSharedPointer<Theme> theme)
{
//    m_status->setTheme(theme);
    update();
}

// static
void DocumentEditor::moveDocumentView(DocumentModel *view, DocumentEditor *editor)
{
    if (view->editor() && editor) {
        view->editor()->removeDocumentView(view);
        editor->addDocumentView(view);
    }
}

void DocumentEditor::addDocumentView(DocumentModel *view)
{
    if (view->editor() == this) {
        qWarning("View is already added to the editor.");
        return;
    }
    m_views.append(view);
    view->setEditor(this);
}

void DocumentEditor::removeDocumentView(DocumentModel *view)
{
    if (view->editor() != this) {
        qWarning("View does not belong to this editor.");
        return;
    }
    view->setEditor(NULL);
    m_views.removeOne(view);
}

void DocumentEditor::setActiveDocumentView(DocumentModel *view)
{
    if (view == m_dv) {
        return;
    }

    if (view && view->editor() != NULL && view->editor() != this) {
        Q_ASSERT(false && "View does not belong to this editor.");
        qWarning("View does not belong to this editor.");
        return;
    }

    qreal normal_gutter_width = 0;

    if (m_dv) {
        m_dv->endCompletion();
        disconnect(m_layout, 0, this, 0);
        normal_gutter_width = m_layout->normalGutterWidth();
        delete m_layout;
        m_layout = NULL;
        disconnect(m_dv, 0, this, 0);
        m_dv->document()->setActiveHint(false);
    }

    DocumentModel *old = m_dv;
    m_dv = view;

    if (m_dv) {
        m_layout = new DocumentEditorLayout(this, m_dv);
        connect(m_layout, &DocumentEditorLayout::normalGutterWidthChanged, this, &DocumentEditor::normalGutterWidthChanged);
        if (m_layout->normalGutterWidth() != normal_gutter_width) {
            emit normalGutterWidthChanged();
        }

        connect(m_dv, SIGNAL(configured(DocumentOptions)), this, SLOT(configured(DocumentOptions)));
        connect(m_dv, SIGNAL(textChanged(const DocumentEditEvent*)), this, SLOT(textChanged(const DocumentEditEvent*)));
        connect(m_dv, SIGNAL(formatChanged(int)), this, SLOT(formatChanged(int)));
        connect(m_dv,
                SIGNAL(cursorChanged(DocumentRange, DocumentRange)),
                this,
                SLOT(cursorChanged(DocumentRange,DocumentRange)));
        connect(m_dv, SIGNAL(completionBegin()), this, SLOT(completionBegin()));
        connect(m_dv, SIGNAL(completionEnd()), this, SLOT(completionEnd()));
        connect(m_dv, SIGNAL(completionUpdated()), this, SLOT(completionUpdated()));

        update();
        scrollToLastBlock();
        emit documentViewChanged(old, m_dv);
        m_dv->document()->setActiveHint(true);
    }

   // updatePreferredHeight();
}

QRectF DocumentEditor::viewport()
{
    return rect();
}

void DocumentEditor::scrollTo(qreal x, qreal y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void DocumentEditor::scrollBy(qreal dx, qreal dy)
{
    m_layout->scrollBy(dx, dy);
    update();
}

void DocumentEditor::scrollToRange(const DocumentRange &r)
{
    m_layout->scrollToRange(r);
    update();
}

void DocumentEditor::scrollToPosition(const DocumentPosition &p)
{
    DocumentRange r(p, p);
    scrollToRange(r);
}

//
// DocumentView events
//

void DocumentEditor::configured(const DocumentOptions &previous)
{
    m_layout->configured(previous);
    updateGeometry();
}

void DocumentEditor::beforeTextChanged()
{
    m_was_last_block_visible = isLastBlockVisible();
}

void DocumentEditor::beforeFormatChanged()
{
    m_was_last_block_visible = isLastBlockVisible();
}

void DocumentEditor::textChanged(const DocumentEditEvent *event)
{
    Q_UNUSED(event);
    m_layout->textChanged(event);

    // updatePreferredHeight();
    scrollToLastBlock();
}

void DocumentEditor::formatChanged(int block)
{
    Q_UNUSED(block);
    m_layout->formatChanged(block);

    scrollToLastBlock();
}

void DocumentEditor::cursorChanged(const DocumentRange &o, const DocumentRange &n)
{
    if ((m_scroll_policy == ScrollPolicy::Default) || (hasFocus())) {
        scrollToRange(n);
    }

    m_layout->cursorChanged(o, n);
}

//
// Editor events
//

void DocumentEditor::moveEvent(QMoveEvent *)
{
    updateCompletionWindowPosition();
}

void DocumentEditor::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);

    if (!m_dv) {
        return;
    }

    QRectF document_rect(rect());
    m_layout->resize(document_rect.width(), document_rect.height());

    scrollToLastBlock();
    updateCompletionWindowPosition();
}

void DocumentEditor::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter p(this);

    if (m_layout == NULL) {
        QTextOption option;
        option.setAlignment(Qt::AlignCenter);
        p.setPen(Qt::white);
        p.drawText(rect(), "No view.", option);
        p.fillRect(rect(), Qt::black);
    } else {
        m_layout->paint(&p);
    }

//    if (hasFocus()) {
//        p.setPen(QColor(255, 0, 0, 255));
//        p.drawRect(rect().adjusted(0, 0, -1, -1));
//    }
}

//
// Keyboard input
//

void DocumentEditor::keyPressEvent(QKeyEvent *event)
{
    if (!m_dv) {
        return;
    }

    // PERFORMANCE_TIMER(t, __FUNCTION__);
    // qDebug() << __FUNCTION__ << event->text();
    if (!event->text().isEmpty()) {
        DocumentEdit edit = m_dv->edit();
        m_dv->insertText(edit, event->text());
    }
}

//
// Mouse input
//

void DocumentEditor::wheelEvent(QWheelEvent *event)
{
    if (!m_dv) {
        return;
    }

    // TODO(cohen) Use pixelDelta on MAC OS X.
    // qDebug() << "wheel" << event->angleDelta();
    qreal amount = -((qreal)event->angleDelta().y())/120.0;

    // TODO(cohen) Scroll by 4 lines.
    scrollBy(0, (amount*80.0f));
    // scrollBy(0, amount*1);
}

//
//
//

void DocumentEditor::focusInEvent(QFocusEvent *e)
{
    // qDebug() << __FUNCTION__;
    QWidget::focusInEvent(e);

    emit focused();
}

void DocumentEditor::focusOutEvent(QFocusEvent *e)
{
    // qDebug() << __FUNCTION__;
    QWidget::focusOutEvent(e);
}

// TODO: This is pure bullshit.
void DocumentEditor::updatePreferredHeight()
{
    if (!m_layout) {
        return;
    }

    if (!m_layout->isValid()) {
        return;
    }

    int preferred = 0;
    QRect desktop_rect = QApplication::desktop()->screenGeometry();
    int max_lines = desktop_rect.height() / m_dv->lineHeight();
    int first = m_layout->viewportRange().first.block;
    int last = qMin(m_dv->blockCount(), first + max_lines + 1);
    Q_ASSERT(first <= last);
    for (int i = first; i < last; i++) {
        preferred += m_dv->blockHeight(i);
    }

    if (preferred != m_height_preferred) {
        m_height_preferred = preferred;
        updateGeometry();
        if (objectName() == "editor.log") {
            // qDebug() << __FUNCTION__ << m_height_preferred;
        }
    }
}

bool DocumentEditor::isLastBlockVisible()
{
    if (m_dv) {
        return true;
    }

    return m_layout->isLastBlockVisible();
}

void DocumentEditor::scrollToLastBlock()
{
    if (!m_dv) {
        return;
    }

    if (m_scroll_policy == ScrollPolicy::Last && !hasFocus())
    {
        m_layout->scrollPositionToBottom(m_dv->documentEnd());
        // scrollToRange(DocumentRange(m_dv->documentEnd()));
    }
}

//
//
//

void DocumentEditor::updateCompletionWindowPosition()
{
    auto completion = m_dv->completionController();
    if (!completion) {
        return;
    }

    scrollToPosition(completion->beginPosition());
    QRect rect = m_layout->documentPositionToRect(completion->beginPosition()).toRect();
    if (!rect.isValid()) {
        rect = m_layout->documentPositionToRect(completion->beginPosition()).toRect();
        if (!rect.isValid()) {
            return;
        }
    }

    m_completion_window->setCompletionRect(QRect(mapToGlobal(rect.topLeft()), rect.size()));
}

void DocumentEditor::completionBegin()
{
    auto completion = m_dv->completionController();

    if (m_completion_window == NULL) {
        m_completion_window = new CompletionWindow(window());
    }
    m_completion_window->setModel(completion->model(), completion->selectionModel());

    updateCompletionWindowPosition();

    m_completion_window->show();
    setFocus(Qt::OtherFocusReason);
}

void DocumentEditor::completionUpdated()
{
    m_completion_window->setFilter(m_dv->completionController()->text());
    updateCompletionWindowPosition();
}

void DocumentEditor::completionEnd()
{
    Q_ASSERT(m_completion_window);
    m_completion_window->setModel(NULL, NULL);
    m_completion_window->hide();
}

//
//
//

DocumentEditorGroup::DocumentEditorGroup(QObject *parent) :
    QObject(parent)
{

}

void DocumentEditorGroup::addEditor(DocumentEditor *editor)
{
    m_editors.append(editor);
    connect(editor, &DocumentEditor::normalGutterWidthChanged, this, &DocumentEditorGroup::updateGutterWidths);
    updateGutterWidths();
}

void DocumentEditorGroup::updateGutterWidths()
{
    qreal gutter_width = 0;
    for (auto editor : m_editors) {
        if (editor->normalGutterWidth() > gutter_width) {
            gutter_width = editor->normalGutterWidth();
        }
    }

    for (auto editor : m_editors) {
        editor->setGutterWidth(gutter_width);
    }
}
