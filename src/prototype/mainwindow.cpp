#include "precompiled.h"
#include "application.h"
#include "configuration.h"

#include "documenteditor.h"
#include "documentmodelview.h"
#include "consolemodel.h"
#include "consolemodelview.h"

#include "windowheader.h"
#include "mainwindow.h"

#include "frameless.h"

#define HANDLE_WIDTH (0)

MainWindow::MainWindow(Application *) :
    QMainWindow(NULL),
    m_console_panel(NULL)
{
    qObject = this;
}

MainWindow::~MainWindow()
{

}

void MainWindow::init()
{
    m_main_splitter = new QSplitter(Qt::Vertical, this);
    m_main_splitter->setChildrenCollapsible(false);
    m_main_splitter->setHandleWidth(HANDLE_WIDTH);
    setCentralWidget(m_main_splitter);

//    auto header = new WindowHeader(this);
//    m_main_splitter->addWidget(header);

    m_content_splitter = new QSplitter(Qt::Horizontal, this);
    m_content_splitter->setHandleWidth(HANDLE_WIDTH);
    m_main_splitter->addWidget(m_content_splitter);

    Q_ASSERT(m_console_panel == NULL);
    m_console_panel = new Panel(m_content_splitter);
    m_console_panel->setObjectName("console");
    attachPanel(m_console_panel);
    m_main_splitter->addWidget(m_console_panel);
    m_console_panel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    // Header
    // m_main_splitter->setStretchFactor(0, 0);
    // Editor area
    m_main_splitter->setStretchFactor(0, 1);
    // Console area
    m_main_splitter->setStretchFactor(1, 0);

    // m_console_panel->resize(m_console_panel->width(), m_console_panel-);

    Frameless::init(this);
    resize(960, 560);
}

//bool MainWindow::event(QEvent *event)
//{
//    if (Frameless::event(this, event)) {
//        return true;
//    }
//    return QMainWindow::event(event);
//}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (Frameless::nativeEvent(this, eventType, message, result)) {
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::scope(ScopePath *o_path)
{
    o_path->push("window", this);
}

ConfigurationObserver *MainWindow::scopeParent()
{
    return Application::instance();
}

void MainWindow::configure(QJsonObject &object)
{

}

Panel *MainWindow::createDefaultPanel()
{
    Q_ASSERT(m_content_splitter->count() == 0);
    if (m_content_splitter->count() == 0) {
        auto panel = new Panel(m_content_splitter);
        attachPanel(panel);
        m_content_splitter->addWidget(panel);
        return panel;
    }
    return NULL;
}

Panel* MainWindow::addPanel(Panel *pivot, Direction direction)
{
    Qt::Orientation orientation = direction == Direction::Left || direction == Direction::Right ? Qt::Horizontal : Qt::Vertical;

    QSplitter *pivot_splitter = dynamic_cast<QSplitter *>(pivot->parentWidget());
    Q_ASSERT(pivot_splitter);
    Q_ASSERT(pivot_splitter != m_main_splitter);
    if (!pivot_splitter) {
        qWarning() << __FUNCTION__ << "Panel doesn't support splitting.";
        return NULL;
    }

    int pivot_size;
    if (pivot_splitter->orientation() == Qt::Horizontal) {
        pivot_size = pivot_splitter->frameGeometry().width();
    } else {
        pivot_size = pivot_splitter->frameGeometry().height();
    }
    int pivot_index;
    QSplitter *new_splitter = NULL;
    if (pivot_splitter->orientation() != orientation) {
        if (pivot_splitter->count() == 1) {
            // Reuse the splitter we have.
            new_splitter = pivot_splitter;
            new_splitter->setOrientation(orientation);
        } else {
            // Create a new splitter for the pivot and the new panel.
            pivot_index = pivot_splitter->indexOf(pivot);
            new_splitter = new QSplitter(orientation, pivot_splitter);
            pivot_splitter->insertWidget(pivot_index, new_splitter);
            new_splitter->addWidget(pivot);
        }
    } else {
        new_splitter = pivot_splitter;
    }

    auto new_panel = new Panel(new_splitter);
    attachPanel(new_panel);

    pivot_index = new_splitter->indexOf(pivot);
    if (direction == Direction::Right || direction == Direction::Down) {
        pivot_index++;
    }
    new_splitter->insertWidget(pivot_index, new_panel);

    updateSplitterSizes(new_splitter);

    return new_panel;
}

void MainWindow::updateSplitterSizes(QSplitter *splitter)
{
    int w = 0;
    QList<int> sizes(splitter->sizes());
    for (int s : sizes) {
        w += s;
    }

    sizes.clear();
    int q = w / splitter->count();
    for (int i = 0; i < splitter->count(); i++) {
        sizes.append(q);
    }
    splitter->setSizes(sizes);
}

void MainWindow::removeWidget(QWidget *widget)
{
    QSplitter *splitter = dynamic_cast<QSplitter *>(widget->parentWidget());
    if (splitter != m_main_splitter && splitter->count() == 1) {
        removeWidget(widget);
    } else {
        delete widget;
        updateSplitterSizes(splitter);
    }
}

void MainWindow::removePanel(Panel *panel)
{
    detachPanel(panel);
    removeWidget(panel);
}

void MainWindow::attachPanel(Panel *panel)
{
    emit panelAdded(panel);
    Configuration::instance->observe(panel);
}

void MainWindow::detachPanel(Panel *panel)
{
    Configuration::instance->forget(panel);
    emit panelRemoved(panel);
}

//
//
//

void MainWindow::splitterFromJson(QSplitter *parent, const QJsonObject &json)
{
    const QJsonArray jchildren = json["children"].toArray();
    for (auto j : jchildren) {
        const QJsonObject jchild = j.toObject();
        if (jchild["type"] == "QSplitter") {
            int orientation = jchild["orientation"].toInt();
            QSplitter *child = new QSplitter((Qt::Orientation)orientation, parent);
            child->setHandleWidth(HANDLE_WIDTH);
            splitterFromJson(child, jchild);
            parent->addWidget(child);
        } else if (jchild["type"] == "Panel") {
            Panel *panel = new Panel(parent);
            attachPanel(panel);
            parent->addWidget(panel);
            // Do this after the panel has been added to splitters.
            panel->fromJson(jchild, NULL);
        }
    }

    QList<int> sizes;
    const QJsonArray jsizes = json["sizes"].toArray();
    if (jsizes.count() == parent->count()) {
        for (auto j : jsizes) {
            sizes << j.toInt();
        }
    }
    parent->setSizes(sizes);
}

void MainWindow::splitterToJson(QSplitter *parent, QJsonObject &json)
{
    QSplitter *child_splitter;
    Panel *child_panel;
    QJsonArray jchildren;
    for (int i = 0; i < parent->count(); i++) {
        child_splitter = qobject_cast<QSplitter*>(parent->widget(i));
        if (child_splitter) {
            QJsonObject jchild;
            splitterToJson(child_splitter, jchild);
            jchildren.append(jchild);
            continue;
        }
        child_panel = qobject_cast<Panel*>(parent->widget(i));
        if (child_panel) {
            QJsonObject jchild;
            jchild["type"] = "Panel";
            child_panel->toJson(jchild, NULL);
            jchildren.append(jchild);
            continue;
        }
        Q_ASSERT(0 && "Unknown type in splitter.");
    }
    json["type"] = "QSplitter";
    json["children"] = jchildren;

    QList<int> sizes(parent->sizes());
    QJsonArray jsizes;
    for (int i : sizes) {
        jsizes.append(i);
    }
    json["sizes"] = jsizes;
    json["orientation"] = (int)parent->orientation();
}

void MainWindow::loadState(const QJsonObject &json)
{
    QByteArray state(QByteArray::fromBase64(json["state"].toString().toLatin1()));
    restoreGeometry(state);

    splitterFromJson(m_content_splitter, json);
    if (m_content_splitter->count() == 0) {
        createDefaultPanel();
    }

    int console_height = json["consoleHeight"].toInt();
    if (console_height < consolePanel()->minimumSize().height()) {
        consolePanel()->resize(consolePanel()->width(), consolePanel()->minimumSize().height());
    } else {
        consolePanel()->resize(consolePanel()->width(), console_height);
    }
}


void MainWindow::saveState(QJsonObject &json)
{
    json["state"] = QString(saveGeometry().toBase64());
    json["consoleHeight"] = consolePanel()->height();
    splitterToJson(m_content_splitter, json);
}
