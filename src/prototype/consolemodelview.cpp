#include "precompiled.h"

#include "application.h"
#include "luaengine.h"

#include "scopepath.h"
#include "statusline.h"
#include "document.h"
#include "documentmodel.h"
#include "documenteditor.h"

#include "consolemodel.h"
#include "consolemodelview.h"

// Log
// Minimum - 1 block (with possibility to close it)
// Maximum - Essentially anything
// Preffered - Height that shows all blocks

ConsoleModelView::ConsoleModelView(Panel *parent) :
    ModelView(parent)
{
    m_log_editor = new DocumentEditor(this);
    m_log_editor->setObjectName("editor.log");

    m_command_editor = new DocumentEditor(this);

    auto group = new DocumentEditorGroup(this);
    group->addEditor(m_log_editor);
    group->addEditor(m_command_editor);

    m_log_editor->setScrollPolicy(DocumentEditor::ScrollPolicy::Last);

    m_log_editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
    // m_command_editor->setContentSizePolicy(DocumentEditor::ContentSizePolicy::Fit);
    m_command_editor->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_log_editor);
    layout->addWidget(m_command_editor);

    setFocusProxy(m_command_editor);
}

//QSize ConsolePanelView::sizeHint() const
//{
//    QSize size;
//    size.setWidth(0);
//    qreal height = 0;
//    height += m_command_editor->sizeHint().height();
//    height += m_log_editor->sizeHint().height();
//    return size;
//}

//QSize ConsolePanelView::sizeHint() const
//{
//    QSize size;
//    size.setWidth(0);
//    qreal height = 0;
//    height += m_command_editor->minimumSizeHint().height();
//    height += m_log_editor->minimumSizeHint().height();
//    return size;
//}

QSize ConsoleModelView::sizeHint() const
{
    return m_command_editor->sizeHint();
}

QSize ConsoleModelView::minimumSizeHint() const
{
    return m_command_editor->minimumSizeHint();
}

void ConsoleModelView::scopePath(ScopePath *o_path, Model *model)
{
    auto console_model = qobject_cast<ConsoleModel*>(model);
    Q_ASSERT(console_model);

    o_path->push("console", m_model);

    // TODO: In the future this'll be one document model.

    if (m_log_editor->hasFocus()) {
        o_path->push("log");
        console_model->logView()->scopePath(o_path);
        // m_log_editor->activeDocumentView()->scopePath(o_path);
        // o_path->push("document", m_log_editor->activeDocumentView());
    } else if (m_command_editor->hasFocus()){
        o_path->push("command");
        console_model->commandView()->scopePath(o_path);
        // m_command_editor->activeDocumentView()->scopePath(o_path);
        // o_path->push("document", m_command_editor->activeDocumentView());
    }
}

bool ConsoleModelView::supportsModel(Model *model)
{
    return qobject_cast<ConsoleModel*>(model) != NULL;
}

void ConsoleModelView::addModel(Model *model)
{
    m_log_editor->addDocumentView(qobject_cast<ConsoleModel*>(model)->logView());
    m_command_editor->addDocumentView(qobject_cast<ConsoleModel*>(model)->commandView());
}

void ConsoleModelView::removeModel(Model *model)
{
    Q_ASSERT(model != m_model);
    m_log_editor->removeDocumentView(qobject_cast<ConsoleModel*>(model)->logView());
    m_command_editor->removeDocumentView(qobject_cast<ConsoleModel*>(model)->commandView());
}

void ConsoleModelView::setModel(Model *model)
{
    m_model = model;
    if (model) {
        m_log_editor->setActiveDocumentView(qobject_cast<ConsoleModel*>(model)->logView());
        m_command_editor->setActiveDocumentView(qobject_cast<ConsoleModel*>(model)->commandView());
    } else {
        m_log_editor->setActiveDocumentView(NULL);
        m_command_editor->setActiveDocumentView(NULL);
    }
}
