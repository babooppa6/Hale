#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QCommandLineParser>

#include "enums.h"
#include "scopepath.h"
#include "tlua.h"
#include "configurationobserver.h"

class MainWindow;
class ConsoleView;

class Project;
class Document;
class DocumentModel;
class DocumentEditor;

class DocumentManager;
class ProjectManager;
class CommandManager;
struct Command;
class SwitchController;

class GrammarManager;

class ConsoleModel;

class Panel;
class Model;

class StatusLine;

class Configuration;

class PathCompleter;

class LuaEngine;

class Application : public QObject, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit Application(int argc, char *argv[]);
    ~Application();

    void scope(ScopePath *o_path);
    ConfigurationObserver *scopeParent();
    void configure(QJsonObject &object);

    static Application *instance() {
        return s_instance;
    }

    /// Should be always used for loading data
    /// from application directory.
    QString resourcePath();

    QApplication *application()
    { return &m_application; }

    CommandManager *commandManager()
    { return m_command_manager; }

    ProjectManager *projectManager()
    { return m_project_manager; }

    DocumentManager *documentManager()
    { return m_document_manager; }

    GrammarManager *grammarManager()
    { return m_grammar_manager; }

    PathCompleter *pathCompleter()
    { return m_path_indexer; }

    LuaEngine *luaEngine()
    { return m_lua; }

    ConsoleModel *console()
    { return m_console_model; }

    int exec(QCommandLineParser &cli);

    tlua::table lua_controller;


protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static Application *s_instance;
    QApplication m_application;
    LuaEngine *m_lua;
    ConsoleModel *m_console_model;

    GrammarManager *m_grammar_manager;
    DocumentManager *m_document_manager;
    ProjectManager *m_project_manager;
    CommandManager *m_command_manager;
    PathCompleter *m_path_indexer;

    MainWindow *m_main_window;

    //
    // Coroutines
    //

public:
    QTimer *idle_timer;

private slots:
    void idleTimer();

    //
    // Configuration
    //
public:
    void updateFile(const QString &path);
    void loadStylesheet(const QString &path);
    void reload();

    void configure(Configuration *configuration);

private:
    Configuration *m_configuration;


    //
    // DocumentView
    //

public:
    /// Creates new document view.
    DocumentModel *createDocumentModel();
    /// Loads a document view.
    DocumentModel *loadDocumentModel(const QString &path);
    /// Saves document view.
    bool saveDocumentModel(DocumentModel *view);
    bool saveDocumentModel(DocumentModel *view, const QString &path);

signals:

private slots:
    void executeCommands(const QList<Command*> &commands, const ScopePath *scope_path);

private:
    DocumentModel *createDocumentModel(Document *document);

    //
    // Scope
    //

public:
    tlua::userdata lua_focus_scope;

    /// Returns current scopepath.
    const ScopePath *scopePath()
    { return &m_focus_scope; }

    /// Shows switch window for panel.
    void beginSwitch(Panel *panel);
    /// Ends switch window for panel.
    void endSwitch();
    /// Switches to next model.
    void switchCommand(SwitchCommand command);

    /// Finds panel that owns given object.
    Panel *findPanelForObject(QObject *object);
    /// Finds panel that owns given model.
    Panel *findPanelForModel(Model *model);
    /// Finds panel by id.
    Panel *findPanel(const QUuid &id);

    /// Returns index of panel.
    int panelIndex(Panel *panel);
    /// Returns next panel.
    Panel *nextPanel(Panel *panel);
    /// Returns previous panel.
    Panel *previousPanel(Panel *panel);
    /// Closes the panel
    void closePanel(Panel *panel);

    /// Sets active model.
    void setActiveModel(Model *model);
    /// Returns active document view.
    Model *activeModel();
    /// Closes model.
    void closeModel(Model *model);
    /// Moves model to panel.
    Model *moveModel(Model *model, Panel *panel);
    Model *moveModel(Model *model, Direction direction);
    /// Splits given panel.
    Model *splitModel(Model *model, Direction direction);

    /// Always returns a panel.
    Panel *requirePanel();
    /// Returns active panel.
    Panel *activePanel();
    /// Returns active non-console panel.
    Panel *activeNonConsolePanel();
    /// Adds panel
    Panel *addPanel(Panel *pivot);
    /// Removes panel
    void removePanel(Panel *panel);


    /// Returns console panel.
    Panel *consolePanel();

    /// This is called for case the focusing is not working yet.
    void focusPanel(Panel *panel);

    /// Fill path with panel's scope.
    // void makeScopePath(ScopePath *o_path, Panel *panel, Model *model);

    typedef QList<Panel *> Panels;
    Panels panels;

    ConfigurationObserver *modeBeginEvent(const QString &name);
    void modeEndEvent(const QString &name, ConfigurationObserver *object);

signals:
    void historyChanged();

private slots:
    void focusChanged(QWidget *old, QWidget *now);
    void modelChanged(Model *previous, Model *current);
    void panelAdded(Panel *panel);
    void panelRemoved(Panel *panel);

    void documentCreated(Document *document);
    void documentLoaded(Document *document);
    void documentSaved(Document *document);

private:
    SwitchController *m_switch_controller;
    ScopePath m_focus_scope;

    void historyTopChange(Panel *previous, Panel *current);

    //
    // State
    //

public:
    void saveState();
    void loadState();

private:
    void saveState(QJsonObject &j);
    void loadState(const QJsonObject &j);
};

#endif // APPLICATION_H
