#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QVector>

class Project;
class Document;

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(QObject *parent = 0);

//    Project *loadProject(const QString &path);
//    void closeProject();

//    Project *project() {
//        return m_project;
//    }

//    void reload();

    Project *findProject(const QString &path);
    void documentCreated(Document *document);
    void documentLoaded(Document *document);
    void documentSaved(Document *document);

signals:
//    void projectOpened(Project *project);
//    void projectClosed();

private slots:


private:
    typedef QList<Project*> Projects;
    Projects m_projects;

//    // Current project
//    Project *m_project;

//    void installProject();
//    void uninstallProject();
};

#endif // PROJECTMANAGER_H
