#include <QFileInfo>
#include <QDir>

#include "document.h"
#include "project.h"
#include "projectmanager.h"

#define PROJECT_FILE ".hale"

ProjectManager::ProjectManager(QObject *parent) :
    QObject(parent)
{
}

//Project *ProjectManager::loadProject(const QString &path)
//{
//    QFileInfo file(path);
//    QDir dir(file.absoluteDir());

//    bool found = false;
//    do
//    {
//        if (QFile(dir.filePath(PROJECT_FILE)).exists())
//        {
//            found = true;
//            break;
//        }
//    } while (dir.cdUp());

//    Project *project = NULL;

//    if (found) {
//        project = new Project(dir.absolutePath(), this);
//    } else {
//        project = new Project(file.absolutePath(), this);
//    }

//    closeProject();

//    m_project = project;

//    installProject();

//    emit projectOpened(project);

//    return project;
//}

//void ProjectManager::closeProject()
//{
//    if (m_project) {
//        emit projectClosed();
//        uninstallProject();
//        delete m_project;
//        m_project = NULL;
//    }
//}

Project *ProjectManager::findProject(const QString &path)
{
    Q_ASSERT(!path.isEmpty());
    for (auto project : m_projects) {
        if (project->path.startsWith(path)) {
            return project;
        }
    }

    QDir dir(QFileInfo(path).absoluteDir());

    bool found = false;
    do {
        if (QFile(dir.filePath(PROJECT_FILE)).exists()) {
            found = true;
            break;
        }
    } while (dir.cdUp());

    if (found) {
        auto project = Project::createProject(dir.absolutePath(), this);
        if (project != NULL) {
            m_projects.append(project);
        }
        return project;
    }

    return NULL;
}

void ProjectManager::documentLoaded(Document *document)
{
    auto project = findProject(document->path());
    if (project) {
        project->documentLoaded(document);
    }
}

void ProjectManager::documentSaved(Document *document)
{
    auto project = findProject(document->path());
    if (project) {
        project->documentSaved(document);
    }
}

void ProjectManager::documentCreated(Document *document)
{
    if (!document->path().isEmpty()) {
        auto project = findProject(document->path());
        if (project) {
            project->documentCreated(document);
        }
    }
}

//void ProjectManager::reload()
//{
//    uninstallProject();
//    installProject();
//}

//void ProjectManager::installProject()
//{
//    if (m_project) {
//        m_js_engine->evaluateFile(m_project->path() + "/.project.inua");
//    }
//}

//void ProjectManager::uninstallProject()
//{
    // TODO(cohen)
//}
