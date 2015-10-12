#include "precompiled.h"

#include "application.h"

#include "modelmetaregistry.h"
#include "configuration.h"
#include "statusline.h"
#include "scopepath.h"
#include "model.h"

#include "panel.h"

Panel::Panel(QWidget *parent) :
    QWidget(parent),
    id(QUuid::createUuid()),
    m_view(NULL),
    current_model(NULL),
    m_status_line(NULL),
    m_active(false)
{
    qObject = this;

    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_status_line = new StatusLine(this);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_layout->addWidget(m_status_line);

    m_views.append(new DefaultModelView(this));
    m_views.back()->hide();
    setView(m_views.back(), NULL);

    setObjectName("panel");
}

Panel::~Panel()
{
}

void Panel::scope(ScopePath *o_path)
{
    o_path->push("panel", this);
}

void Panel::configure(QJsonObject &object)
{

}

//void Panel::scope(ScopePath *o_path, Model *model)
//{
//    o_path->push("panel", this);
//    ModelView *view;
//    if (model == NULL) {
//        model = current_model;
//    }

//    view = getView(model);
//    if (view) {
//        view->scopePath(o_path, model);
//    }
//}

void Panel::setTitle(const QString &title)
{
    if (title != m_title) {
        m_title = title;
        emit titleChanged();
    }
}

QSize Panel::sizeHint() const
{
    QSize size;
    if (m_view) {
        size = m_view->sizeHint();
    }
    if (hasStatusLine()) {
        size.setHeight(size.height() + m_status_line->sizeHint().height());
    }
    size += QSize(m_layout->margin() * 2, m_layout->margin() * 2);
    return size;
}

QSize Panel::minimumSizeHint() const
{
    QSize size;
    if (m_view) {
        size = m_view->minimumSizeHint();
    }
    if (hasStatusLine()) {
        size.setHeight(size.height() + m_status_line->minimumSizeHint().height());
    }
    size += QSize(m_layout->margin() * 2, m_layout->margin() * 2);
    return size;
}

//QSize Panel::maximumSizeHint() const
//{
//    QSize size;
//    if (m_view) {
//        size = m_view->maximSizeHint();
//    }
//    if (hasStatusLine()) {
//        size.setHeight(size.height() + m_status_line->maximumSizeHint().height());
//    }
//    return size;
//}

void Panel::addModel(Model *model)
{
    Q_ASSERT(models.contains(model) == false);
    auto view = getView(model);
    model->setParent(this);
    if (view) {
        view->addModel(model);
    }
    models.append(model);

    Configuration::instance->observe(model);

    setModel(model);
}

Model *Panel::removeModel(Model *model)
{
    Configuration::instance->forget(model);

    Q_ASSERT(models.contains(model) == true);
    bool was_top = models.last() == model;

    models.removeOne(model);

    if (was_top) {
        if (models.size() == 0) {
            setModel(NULL);
        } else {
            setModel(models.last());
        }
    }

    model->setParent(NULL);

    return model;
}

bool Panel::hasModel(Model *model)
{
    for (auto m : models) {
        if (m == model) {
            return true;
        }
    }
    return false;
}

void Panel::setModel(Model *model)
{
    peekModel(model);

    if (current_model && current_model != models.last()) {
        models.removeOne(current_model);
        models.append(current_model);
    }
}

void Panel::peekModel(Model *model)
{
    if (model == current_model) {
        return;
    }

    ModelView *view = NULL;
    if (model != NULL) {
        bool has = models.contains(model);
        if (model->parent() == NULL && !has) {
            addModel(model);
        } else if (model->parent() != this) {
            qWarning() << "Model does not belog to this panel.";
            return;
        }
    }

    view = getView(model);
    if (view == NULL && model != NULL) {
        qWarning() << "Model cannot be opened as there's no view to open it.";
        return;
    }

    auto old = current_model;
    if (current_model) {
        auto status_line_controller = current_model->statusLineController();
        if (status_line_controller) {
            status_line_controller->uninstall();
        }
        m_status_line->controller = NULL;
        m_status_line->setVisible(false);
    }

    m_status_line->clear();
    current_model = model;
    setView(view, current_model);

    if (current_model)
    {
        m_status_line->controller = current_model->statusLineController();
        if (m_status_line->controller) {
            m_status_line->controller->install(m_status_line);
            m_status_line->setVisible(true);
        } else {
            m_status_line->setVisible(false);
        }
    }

    updateGeometry();
    emit modelChanged(old, current_model);
}

void Panel::setView(ModelView *view, Model *model)
{
    bool had_focus = hasFocus();
    if (m_view == view) {
        m_view->setModel(model);
        return;
    }

    ModelView *old = m_view;
    m_view = view;
    if (old) {
        old->hide();
        old->setModel(NULL);
        disconnect(old, 0, this, 0);
        m_layout->removeWidget(old);
    }
    setFocusProxy(m_view);
    if (m_view) {
        m_view->setModel(model);
        m_view->show();
        m_layout->insertWidget(0, m_view);
        // connect(m_view, &PanelView::focused, this, &Panel::focused);
        if (had_focus) {
            m_view->setFocus();
        }
    }
}

void Panel::setActive(bool active)
{
    if (active != m_active) {
        m_active = active;
        update();
    }
}

//
//
//

void Panel::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
//    layoutView();
}

void Panel::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    if (m_view == NULL) {
        QPainter p(this);
        p.fillRect(rect().adjusted(0, 0, -1, -1), Configuration::theme()->backgroundColor());
    } else {
        QPainter p(this);
        if (m_active) {
            p.setPen(Configuration::theme()->selectionColor());
        } else {
            p.setPen(Configuration::theme()->backgroundColor());
        }
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
}

//void Panel::focusInEvent(QFocusEvent *)
//{
//    qDebug() << __FUNCTION__;
//    emit focusedChanged(true);
//}

//void Panel::focusOutEvent(QFocusEvent *)
//{
//    qDebug() << __FUNCTION__;
//    emit focusedChanged(false);
//}

void Panel::layoutView()
{
    QRect r(rect().adjusted(1, 1, -1, -1));
    qreal widget_height = r.height();
    if (hasStatusLine()) {
        QSize size = m_status_line->sizeHint();
        widget_height -= size.height();
        m_status_line->setGeometry(r.left(), widget_height, r.width(), size.height());
    }
    if (m_view) {
        m_view->setGeometry(r.left(), r.top(), r.width(), widget_height);
    }
    updateGeometry();
}

bool Panel::hasStatusLine() const
{
    return m_status_line->controller && m_status_line->isVisible();
}

void Panel::fromJson(const QJsonObject &json, void *)
{
    id = QUuid(json["id"].toString());
    Q_ASSERT(!id.isNull());

    Model *model = NULL;
    QJsonArray jmodels(json["models"].toArray());
    for (auto jmodel : jmodels) {
        model = ModelMetaRegistry::fromJson(jmodel.toObject(), NULL);
        if (model) {
            addModel(model);
        }
    }
    if (model) {
        setModel(model);
    }
}

void Panel::toJson(QJsonObject &json, void *)
{
    json["id"] = id.toString();
    QJsonArray jmodels;
    for (auto model : models) {
        QJsonObject jmodel;
        ModelMetaRegistry::toJson(model, jmodel);
        jmodels.append(jmodel);
    }
    json["models"] = jmodels;
}

//
//
//

#include "documentmodelview.h"
#include "consolemodelview.h"

ModelView *Panel::getView(Model *model)
{
    for (auto view : m_views) {
        if (view->supportsModel(model)) {
            return view;
        }
    }

    auto view = ModelMetaRegistry::metaFor(model)->createModelView(this);
    Q_ASSERT(view);
    Q_ASSERT(view->supportsModel(model));

    m_views.append(view);
    view->hide();

    return view;
}

//
// PanelView
//

ModelView::ModelView(Panel *panel) :
    QWidget(panel)
{
    setFocusPolicy(Qt::StrongFocus);
}

//void PanelView::focusInEvent(QFocusEvent *)
//{
//    qDebug() << __FUNCTION__;
//    emit focusedChanged(true);
//}

//void PanelView::focusOutEvent(QFocusEvent *)
//{
//    qDebug() << __FUNCTION__;
//    emit focusedChanged(false);
//}

//
// DefaultModelView
//

DefaultModelView::DefaultModelView(Panel *panel) :
    ModelView(panel)
{
    // setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

//QSize DefaultModelView::sizeHint() const
//{
//    return QSize(128, 64);
//}

void DefaultModelView::setModel(Model *)
{
}

bool DefaultModelView::supportsModel(Model *model)
{
    return model == NULL;
}

void DefaultModelView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Configuration::theme()->backgroundColor());
    p.setPen(Configuration::theme()->foregroundDimColor());
    p.drawText(rect(), Qt::AlignCenter, "No model.");
}
