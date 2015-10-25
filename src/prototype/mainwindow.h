#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "enums.h"
#include "configurationobserver.h"

class QSplitter;

class Application;
class DocumentModel;

class Panel;
class ModelView;
class Model;

class DocumentModelView;
class DocumentEditor;
class StatusLine;
class ConsoleModel;
class ConsoleModelView;

class ScopePath;

class MainWindow : public QMainWindow, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit MainWindow(Application *application);
    ~MainWindow();

    void scope(ScopePath *o_path);
    void configure(QJsonObject &object);
    virtual ConfigurationObserver *scopeParent();

    void init();

    Panel *createDefaultPanel();
    Panel *addPanel(Panel *pivot, Direction direction);
    void removePanel(Panel *panel);

    Panel *consolePanel()
    { return m_console_panel; }

    void saveState(QJsonObject &json);
    void loadState(const QJsonObject &json);

protected:
//    bool event(QEvent *event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

signals:
    void panelAdded(Panel *panel);
    void panelRemoved(Panel *panel);
//    void modelChanged(Model *previous, Model *current);

private slots:
//    void panelModelChanged(Model *previous, Model *current);
//    void _panelFocused();
//    void _editorFocused();
//    void _editorClosed();
//    void _editorDocumentViewChanged(DocumentView *old_view, DocumentView *new_view);

private:
    ConsoleModelView *m_console;
    Panel *m_console_panel;
    // typedef QList<Panel *> Panels;
    // Panels m_panels;
    QSplitter *m_main_splitter;
    QSplitter *m_content_splitter;

    void splitterToJson(QSplitter *parent, QJsonObject &json);
    void splitterFromJson(QSplitter *parent, const QJsonObject &json);
    void attachPanel(Panel *panel);
    void detachPanel(Panel *panel);

    void updateSplitterSizes(QSplitter *splitter);
    void removeWidget(QWidget *widget);
};

#endif // MAINWINDOW_H
