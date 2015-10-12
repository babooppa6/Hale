#include <QDebug>
#include <QCommandLineParser>
#include <QStyleFactory>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "util.h"
#include "objectfactory.h"

#include "configuration.h"

#include "luaengine.h"
#include "lua_scopepath.h"

#include "projectmanager.h"
#include "commandmanager.h"
#include "pathcompleter.h"

#include "document.h"
#include "project.h"
#include "grammar.h"

#include "scopepath.h"

#include "mainwindow.h"
#include "documenteditor.h"

#include "panel.h"
#include "documentmodel.h"
#include "documentmodelmeta.h"
#include "consolemodel.h"
#include "consolemodelmeta.h"

#include "switchcontroller.h"

#include "application.h"

Application *Application::s_instance = NULL;

#include "modelmetaregistry.h"

Application::Application(int argc, char *argv[]) :
    m_application(argc, argv),
    idle_timer(NULL),
    m_main_window(NULL),
    m_command_manager(NULL),
    m_grammar_manager(NULL),
    m_document_manager(NULL),
    m_lua(NULL),
    m_console_model(NULL),
    m_switch_controller(NULL)
{
    connect(&m_application, &QApplication::focusChanged, this, &Application::focusChanged);
    Q_ASSERT(s_instance == NULL);
    s_instance = this;

    m_configuration = new Configuration(this);

    m_application.installEventFilter(this);

    QApplication::setOrganizationName("Martin Cohen");
    QApplication::setOrganizationDomain("coh.io");
    QApplication::setApplicationName("Hale");

    // loadStylesheet(resourcePath());

    m_grammar_manager = new GrammarManager(this);
    m_document_manager = new DocumentManager(this);
    m_project_manager = new ProjectManager(this);
    m_command_manager = new CommandManager(this);
    m_path_indexer = new PathCompleter(this);
    m_lua = new LuaEngine(this);

    connect(m_command_manager, &CommandManager::executeCommands, this, &Application::executeCommands);

    connect(m_document_manager, &DocumentManager::documentCreated, this, &Application::documentCreated);
    connect(m_document_manager, &DocumentManager::documentLoaded, this, &Application::documentLoaded);
    connect(m_document_manager, &DocumentManager::documentSaved, this, &Application::documentSaved);

    static DocumentModelMeta document_model_meta(m_document_manager);
    ModelMetaRegistry::addMeta<DocumentModel>(&document_model_meta);

    static ConsoleModelMeta console_model_meta(m_lua);
    ModelMetaRegistry::addMeta<ConsoleModel>(&console_model_meta);

    using namespace std::placeholders;
    tlua::settings.pcall = std::bind(&LuaEngine::pcall, m_lua, _1, _2, _3);

    int icons_font_id = QFontDatabase::addApplicationFont(":/images/inuaicons.ttf");
    Q_ASSERT(icons_font_id != -1);
    // loadCommands();
}

Application::~Application()
{
    delete m_switch_controller;
    // ConsoleHandler::release();
}

void Application::scope(ScopePath *o_path)
{
    o_path->push("app", this);
}

ConfigurationObserver *Application::scopeParent()
{
    return NULL;
}

void Application::configure(QJsonObject &object)
{
    Q_UNUSED(object);
}

//void Application::reloadConfiguration()
//{
//    if (m_lua->execute(resourcePath() + "/config.lua", 1) == false) {
//        return;
//    }

//    auto options(tlua::pop<tlua::table>(m_lua->L));

//    options.forEach([&] (const char *key, int type) {
//        if (type == LUA_TTABLE) {
//            Entry entry;
//            entry.selector = key;
//            table_to_json(options.get<tlua::table>(key), &entry.options);
//            list << entry;
//        }
//    });
//    qDebug() << __FUNCTION__;

//    ScopePath scope;
//    ScopeScore score;
//    for (auto panel : panels) {
//        for (auto model : panel->models) {
//            makeScopePath(&scope, panel, model);
//            QJsonObject o;
//            for (auto entry : list) {
//                if (scope.match(entry.selector, &score)) {
//                    merge(&o, &entry.options);
//                }
//            }
//            model->configure(o);
//        }
//    }
//}

#include "finderboyermoore.h"

int Application::exec(QCommandLineParser &cli)
{
    idle_timer = new QTimer(this);
    connect(idle_timer, SIGNAL(timeout()), this, SLOT(idleTimer()));

    m_lua->init();

    m_console_model = new ConsoleModel(NULL, m_lua);
    lua_focus_scope = lua_scopepath_wrap(m_lua->L, &m_focus_scope);

    // TODO: Need to implement async load. Until then, this must happen before document manager loads.
    m_grammar_manager->load(resourcePath());

    m_main_window = new MainWindow(this);
    connect(m_main_window, &MainWindow::panelAdded, this, &Application::panelAdded);
    connect(m_main_window, &MainWindow::panelRemoved, this, &Application::panelRemoved);

    m_main_window->init();
    m_configuration->load();

    m_main_window->consolePanel()->setModel(console());

    loadState();
    // TODO: Process cli.positionalArguments()

    m_lua->execute(resourcePath() + "/hale.lua");

//    m_document_manager->loadState(resourcePath());
//    if (m_document_manager->documentCount()) {
//        // NOTE(cohen) This might load the project file,
//        // so the JS engine has to be initialized.
//        openDocument(m_document_manager->document(0));
//    }

//    splitModel(activeModel(), Qt::Horizontal);
    m_main_window->show();

//    DocumentView *view = qobject_cast<DocumentView*>(activeModel());
//    FinderBoyerMoore finder("hello", [&](int pos) {
//        return view->document()->charAt(pos);
//    });

//    {
//        auto e = view->edit();
//        view->insertText(e, "Lorem ipsum dolor sit amet hello.");
//    }

//    FinderBoyerMoore::Result result;
//    if (finder.find(0, view->document()->length(), 0, view->document()->length(), false, &result)) {
//        qDebug() << "FOUND!" << result.begin << result.end;
//        view->setSelection(result.begin, result.end);
//    }

    int ret = m_application.exec();

    saveState();

    // delete m_js_engine;
    delete m_main_window;
    m_main_window = NULL;

    return ret;
}

void Application::idleTimer()
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>("idle"));
    if (f.empty()) {
        return;
    }

    tlua::push(f.L, lua_controller);

    f.invoke(1, 0);
}

void Application::executeCommands(const QList<Command *> &commands, const ScopePath *)
{
    if (lua_controller.empty()) {
        return;
    }

    tlua::function f(lua_controller.get<tlua::function>("execute"));
    if (f.empty()) {
        return;
    }

    tlua::table lua_commands(tlua::table::create(f.L, commands.size(), 0));
    for (Command *command : commands) {
        if (!command->lua_command.empty()) {
            lua_commands.append(command->lua_command);
        }
    }

    m_focus_scope.dump();

    tlua::push(f.L, lua_controller);
    tlua::push(f.L, lua_commands);
    tlua::push(f.L, lua_focus_scope);

    // luascopepath_push(f.L, scope_path);

    f.invoke(3, 0);
}

void Application::reload()
{
    auto p = resourcePath();
    // m_js_engine->load(p);
    m_grammar_manager->load(p);
    // m_project_manager->reload();

    loadStylesheet(p);
}

//
//
//

bool Application::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
//        if (event->type() == QEvent::KeyRelease) {
//            _CrtDbgBreak();
//        }
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        auto f = ConfigurationObserver::find(m_application.focusWidget());
        m_focus_scope.make(f);
        // m_focus_scope.dump();
        if (m_command_manager->handleKeyEvent(obj, key_event, &m_focus_scope)) {
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}

//
// Configuration
//

void Application::updateFile(const QString &path)
{
    if (m_configuration->update(path)) {
        return;
    } else {
        m_grammar_manager->reloadGrammar(path);
    }
}

void Application::loadStylesheet(const QString &path)
{
    QString style_sheet;
    if (Util::loadStringFromFile(&style_sheet, path + "/styles/style.css")) {
        m_application.setStyleSheet(style_sheet);
    }
}

//void Application::configure()
//{
//    for (auto panel : panels) {
//        configurePanel(panel);
//    }
//}

//void Application::configurePanel(Panel *panel)
//{
//    for (auto model : panel->models) {
//        configureModel(model);
//    }
//}

//void Application::configureModel(Model *model)
//{
//    // TODO: findPanelForModel() is suboptimal here.
//    ScopePath scope;
//    makeScopePath(&scope, findPanelForModel(model), model);
//    QJsonObject o = m_configuration->get(&scope);
//    model->configure(o);
//}

//    m_configuration = new Configuration(this);
//    m_configuration->setUiFont(m_application.font());
//    m_configuration->setTheme(QSharedPointer<Theme>(new Theme));

//#if defined(Q_OS_MAC)
//    QFont font("Nitti", 16);
//#elif defined(Q_OS_WIN)
//    QFont font("Consolas", 11);
//    // -platform windows:fontengine=freetype
//    // QFont font("Nitti", 11);
//    // QFont font("Fira Mono Medium");
//    // font.setPointSizeF(11);
//    // font.setWeight(57);
//#endif
//    font.setFixedPitch(true);
//    font.setHintingPreference(QFont::PreferFullHinting);
//    //font.setHintingPreference(QFont::PreferVerticalHinting);
//    // font.setWeight(QFont::DemiBold);
//    m_configuration->setEditorFont(font);

//    connect(m_configuration, &Configuration::uiFontChanged, this, [&](QFont font, QFontMetrics) {
//        m_application.setFont(font);
//    });

//    // m_application.setStyle(QStyleFactory::create("fusion"));
//    auto theme = m_configuration->theme();
//    QPalette palette;
//    palette.setColor(QPalette::Window, theme->backgroundColor());
//    palette.setColor(QPalette::WindowText, theme->foregroundColor());
//    palette.setColor(QPalette::Base, theme->backgroundColor());
//    palette.setColor(QPalette::AlternateBase, theme->backgroundColor());
//    palette.setColor(QPalette::Text, theme->foregroundColor());
//    palette.setColor(QPalette::BrightText, theme->foregroundBrightColor());
//    palette.setColor(QPalette::Highlight, theme->selectionColor());
//    m_application.setPalette(palette);

//
// State
//

void Application::saveState()
{
    QFile file(resourcePath() + "/state.json");
    if (file.open(QIODevice::WriteOnly))
    {
        QJsonObject j;
        saveState(j);
        file.write(QJsonDocument(j).toJson());
    }
    m_document_manager->saveState();
}

void Application::loadState()
{
    // TODO: Loading UI with models is probably not a very good idea.
    // The models need to be in the panels to apply configuration.
    QDir storage_dir(resourcePath());
    storage_dir.mkpath("state");
    storage_dir.cd("state");
    m_document_manager->storage_path = storage_dir.absolutePath();
    m_document_manager->loadState();
    QFile file(resourcePath() + "/state.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument j = QJsonDocument::fromJson(file.readAll());
        file.close();
        loadState(j.object());
    }
}

void Application::saveState(QJsonObject &j)
{
    m_main_window->saveState(j);
    QJsonArray jhistory;
    for (auto panel : panels) {
        jhistory.append(panel->id.toString());
    }
//    auto it = m_panels_history.end();
//    while (it != m_panels_history.begin()) {
//        it--;
//        jhistory.append((*it)->id.toString());
//    }
    j["history"] = jhistory;
    j["console"] = consolePanel()->id.toString();
}

void Application::loadState(const QJsonObject &j)
{
    m_main_window->loadState(j);
    auto console_id = QUuid(j["console"].toString());
    if (console_id != NULL) {
        consolePanel()->id = console_id;
    }
    QJsonArray jhistory(j["history"].toArray());
    for (auto jpanel : jhistory) {
        QUuid id(jpanel.toString());
        if (!id.isNull()) {
            auto panel = findPanel(id);
            if (panel != NULL) {
                focusPanel(panel);
            }
            // m_panels_history.append(findPanel(id));
        }
    }
}

//
// Project
//

//Project *Application::project()
//{
//    return m_project_manager->project();
//}

//void Application::openProject(const QString &path)
//{
//    m_project_manager->loadProject(path);
//}

//void Application::closeProject()
//{
//    m_project_manager->closeProject();
//}

//
// Document
//

//Document *Application::createDocument()
//{
//    return m_document_manager->newDocument();
//}

//Document *Application::loadDocument(const QString &path)
//{
//    return m_document_manager->loadDocument(path);
//}

//bool Application::saveDocument(Document *document)
//{
//    return m_document_manager->saveDocument(document);
//}

//bool Application::saveDocument(Document *document, const QString &path)
//{
//    return m_document_manager->saveDocument(document, path);
//}

void Application::documentCreated(Document *document)
{
    m_project_manager->documentCreated(document);
    m_grammar_manager->observeDocument(document);
}

void Application::documentLoaded(Document *document)
{
    m_project_manager->documentLoaded(document);
    m_grammar_manager->observeDocument(document);
}

void Application::documentSaved(Document *document)
{
    m_project_manager->documentSaved(document);
}

//
// DocumentView
//

DocumentModel *Application::createDocumentModel(Document *document)
{
    Q_ASSERT(document);
    if (document) {
        // The parent will be set in the Panel.
        auto view = new DocumentModel(document);
        return view;
    }
    return NULL;
}

DocumentModel *Application::createDocumentModel()
{
    auto document = m_document_manager->newDocument();
    if (document) {
        return createDocumentModel(document);
    }
    return NULL;
}

DocumentModel *Application::loadDocumentModel(const QString &path)
{
    auto document = m_document_manager->loadDocument(path);
    if (document) {
        return createDocumentModel(document);
    }
    return NULL;
}

bool Application::saveDocumentModel(DocumentModel *model)
{
    return m_document_manager->saveDocument(model->document());
}

bool Application::saveDocumentModel(DocumentModel *model, const QString &path)
{
    return m_document_manager->saveDocument(model->document(), path);
}

//
// Context management
//

struct switch_mode
{
    SwitchController *switch_controller;
};

ConfigurationObserver *Application::modeBeginEvent(const QString &name)
{
    if (name == "switch") {
        if (m_switch_controller == NULL) {
            m_switch_controller = new SwitchController(this);
            Configuration::instance->observe(m_switch_controller);
        }
        m_switch_controller->show(activePanel());
        return m_switch_controller;
    }
    return NULL;
}

void Application::modeEndEvent(const QString &name, ConfigurationObserver *object)
{
    Q_UNUSED(object);
    if (name == "switch") {
        m_switch_controller->hide();
    }
}

// TODO: This should be implemented on switch controller?
// - I need a non-switch version of this. (without the UI)
void Application::switchCommand(SwitchCommand command)
{
    if (m_switch_controller && m_switch_controller->isActive()) {
        m_switch_controller->command(command);
    }
}

//
//
//

Panel *Application::findPanelForObject(QObject *object)
{
    Panel *panel;
    while (object)
    {
        panel = qobject_cast<Panel*>(object);
        if (panel) {
            return panel;
        }
        object = object->parent();
    }
    return NULL;
}

Panel *Application::findPanelForModel(Model *model)
{
    for (Panel *panel : panels) {
        if (panel->hasModel(model)) {
            return panel;
        }
    }
    return NULL;
}

Panel *Application::findPanel(const QUuid &id)
{
    Q_ASSERT(!id.isNull());
    for (Panel *panel : panels) {
        if (panel->id == id) {
            return panel;
        }
    }
    return NULL;
}

void Application::setActiveModel(Model *model)
{
    auto panel = findPanelForModel(model);
    if (panel == NULL) {
        panel = requirePanel();
        Q_ASSERT(panel);
    }
    panel->setModel(model);
    focusPanel(panel);
}

Panel *Application::requirePanel()
{
    Panel *panel = NULL;
    panel = panels.last(); // findNonConsolePanel(&m_panels_history);
    if (panel) {
        return panel;
    }
//    panel = findNonConsolePanel(&m_panels);
//    if (panel) {
//        return panel;
//    }
    panel = m_main_window->createDefaultPanel();
    Q_ASSERT(panel);
    return panel;
}

//Panel *Application::findNonConsolePanel(Panels *panels)
//{
//    Panels::iterator it = panels->end();
//    while (it != panels->begin()) {
//        it--;
//        if ((*it) != m_main_window->consolePanel()) {
//            return *it;
//        }
//    }
//    return NULL;
//}

int Application::panelIndex(Panel *panel)
{
    for (int i = 0; i < panels.count(); i++) {
        if (panels[i] == panel) {
            return i;
        }
    }
    return -1;
}

Panel *Application::nextPanel(Panel *panel)
{
    if (panels.count() == 1) {
        return panels.first();
    }

    int i = panelIndex(panel);
    Q_ASSERT(i != -1);
    if (i == 0) {
        return panels.last();
    }
    return panels[i - 1];

//    auto console_panel = consolePanel();
//    int i = panelIndex(panel);
//    while (i > 0) {
//        i--;
//        if (m_panels_history[i] != console_panel) {
//            return m_panels_history[i];
//        }
//    }
//    return m_panels_history.last();
}

Panel *Application::previousPanel(Panel *panel)
{
    if (panels.count() == 1) {
        return panels.first();
    }

    int i = panelIndex(panel);
    Q_ASSERT(i != -1);
    if (i == panels.count() - 1) {
        return panels.first();
    }
    return panels[i + 1];

//    int i = m_panels_history.count() - 1;
//    for (; i >= 0; i--) {
//        if (m_panels_history[i] == panel) {
//            i = i + 1;
//            if (i < 0) {
//                i = m_panels_history.count() - 1;
//            } else {
//                i = i % m_panels_history.count();
//            }
//            return m_panels_history[i];
//        }
//    }
//    return m_panels_history.first();
}

void Application::closePanel(Panel *panel)
{
    m_main_window->removePanel(panel);
}

Panel *Application::activePanel()
{
    return panels.isEmpty() ? NULL : panels.last(); // findPanelForObject(m_application.focusObject());
}

Panel *Application::activeNonConsolePanel()
{
    auto console_panel = consolePanel();
    Panels::iterator it = panels.end();
    while (it != panels.begin()) {
        it--;
        if ((*it) != console_panel) {
            return *it;
        }
    }
    return NULL;
}

Model *Application::activeModel()
{
    auto panel = activePanel();
    if (!panel) {
        return NULL;
    }
    return panel->model();
}

void Application::closeModel(Model *model)
{
    auto panel = findPanelForModel(model);
    // Q_ASSERT(panel);
    if (!panel) {
        qWarning() << __FUNCTION__ << "Panel for model was not found.";
        return;
    }
    panel->removeModel(model);
    model->close();
    delete model;
}

Model *Application::splitModel(Model *model, Direction direction)
{
    if (model == NULL) {
        qWarning() << __FUNCTION__ << "Model was NULL.";
        return NULL;
    }

    auto panel = findPanelForModel(model);
    if (!panel) {
        qWarning() << __FUNCTION__ << "No panel found for model.";
        return NULL;
    }

    auto new_panel = m_main_window->addPanel(panel, direction);
    if (!new_panel) {
        qWarning() << __FUNCTION__ << "Unable to split panel.";
    }

    auto new_model = model->split(NULL);
    if (new_model == NULL) {
        qWarning() << __FUNCTION__ << "Model doesn't support splitting.";
        return NULL;
    }

    new_panel->setModel(new_model);

//    focusPanel(new_panel);

    return new_model;
}

//
// Context signals
//

void Application::panelAdded(Panel *panel)
{
    focusPanel(panel);
    // m_panels_history.append(panel);
    // m_panels.append(panel);
}

void Application::panelRemoved(Panel *panel)
{
    Panel *last = panels.last();

    panels.removeOne(panel);
    // m_panels.removeOne(panel);

    if (last == panel) {
        historyTopChange(last, panels.empty() ? NULL : panels.last());
    }
}

Panel *Application::consolePanel()
{
    return findPanelForObject(m_console_model);
}

void Application::focusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    auto o = ConfigurationObserver::find(now);
    if (o) {
        m_focus_scope.make(o);
    }

    auto focused_panel = findPanelForObject(now);
    if (focused_panel == NULL) {
        return;
    }

    focusPanel(focused_panel);
}

void Application::modelChanged(Model *, Model *)
{
    emit historyChanged();
}

void Application::focusPanel(Panel *panel)
{
    Panel *previous = panels.isEmpty() ? NULL : panels.last();
    if (previous == panel) {
        return;
    }

    if (previous) {
        previous->setActive(false);
    }

    if (panel != NULL) {
        panel->setActive(true);
        panels.removeOne(panel);
        panels.append(panel);

        qDebug() << __FUNCTION__ << (panel ? panel->objectName() : "null");
        historyTopChange(previous, panel);

        if (panel && !panel->hasFocus()) {
            panel->setFocus();
        }
    }
}

// TODO: Get rid of this, I don't like it.
void Application::historyTopChange(Panel *previous, Panel *current)
{
    Model *previous_model = NULL;
    Model *current_model = NULL;

    if (previous) {
        previous_model = previous->model();
        disconnect(previous, 0, this, 0);
    }
    if (current) {
        current_model = current->model();
        connect(current, SIGNAL(modelChanged(Model*,Model*)), this, SLOT(modelChanged(Model*,Model*)));
    }

    // emit historyChanged();

    modelChanged(previous_model, current_model);
}

QString Application::resourcePath()
{
    // QString p = QFileInfo(QFileInfo(__PROJECT__).absolutePath() + "/../Data").absoluteFilePath();
#if defined(Q_OS_WIN)
    QString p = QFileInfo(m_application.applicationDirPath()).absoluteFilePath();
#elif defined(Q_OS_DARWIN)
    QString p = QFileInfo(QFileInfo(__PROJECT__).absolutePath() + "/../Data").absoluteFilePath();
#endif
    return p;
}
