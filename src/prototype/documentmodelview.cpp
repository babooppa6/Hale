#include "precompiled.h"
#include "scopepath.h"
#include "statusline.h"
#include "documenteditor.h"
#include "documentmodelview.h"

DocumentModelView::DocumentModelView(Panel *panel) :
    ModelView(panel)
{
    m_editor = new DocumentEditor(this);
    m_editor->setObjectName("editor");
    auto layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_editor);
    setFocusProxy(m_editor);
}

QSize DocumentModelView::sizeHint() const
{
    return m_editor->sizeHint();
}

QSize DocumentModelView::minimumSizeHint() const
{
    return m_editor->minimumSizeHint();
}

void DocumentModelView::scopePath(ScopePath *o_path, Model *model)
{
    // o_path->push("view.document", this);
    if (model) {
        model->scopePath(o_path);
    }
//    if (m_editor->activeDocumentView()) {
//        m_editor->activeDocumentView()->scopePath(o_path);
//    }
}

bool DocumentModelView::supportsModel(Model *model)
{
    return qobject_cast<DocumentModel*>(model) != NULL;
}

void DocumentModelView::addModel(Model *model)
{
    auto view = qobject_cast<DocumentModel*>(model);
    Q_ASSERT(view);
    if (!view) {
        return;
    }

    m_editor->addDocumentView(view);
}

void DocumentModelView::removeModel(Model *model)
{
    auto document_model = qobject_cast<DocumentModel*>(model);
    Q_ASSERT(document_model);
    if (!document_model) {
        return;
    }

    m_editor->removeDocumentView(document_model);
}

void DocumentModelView::setModel(Model *model)
{
    if (model == NULL) {
        m_editor->setActiveDocumentView(NULL);
    } else {
        auto view = qobject_cast<DocumentModel*>(model);
        Q_ASSERT(view);
        if (!view) {
            return;
        }
        m_editor->setActiveDocumentView(view);
    }
}

//Panel *DocumentPanelView::split()
//{
//    auto new_panel = new DocumentPanelView(NULL);

//    auto view = m_editor->activeDocumentView();
//    if (view) {
//        auto new_view = view->split(NULL);
//        new_panel->editor()->addDocumentView(new_view);
//        new_panel->editor()->setActiveDocumentView(new_view);
//    }
//    return new_panel;
//}

//bool DocumentPanelView::hasModel(QObject *model)
//{
//    if (m_editor->activeDocumentView() == model) {
//        return true;
//    }
//    return false;
//}
