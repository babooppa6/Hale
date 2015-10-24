#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QFont>
#include <QFontMetrics>
#include "scopepath.h"

#include <functional>

#include "theme.h"

class ConfigurationObserver;

class Configuration : public QObject
{
    Q_OBJECT
public:
    static Configuration *instance;

    explicit Configuration(QObject *parent = 0);

    /// Loads a configuration file.
    // void load(const QString &path);

    void setTheme(QSharedPointer<Theme> theme);
    static QSharedPointer<Theme> theme()
    { return instance->m_theme; }

    void update(ConfigurationObserver* target);
    void observe(ConfigurationObserver* target);
    bool forget(ConfigurationObserver* target);

    // TODO: QStringList root_files;

//    void setUiFont(QFont font);
//    static QFont uiFont()
//    { return instance()->m_font_ui; }
//    void setEditorFont(QFont font);
//    static QFont editorFont()
//    { return instance()->m_font_editor; }
//    static QFontMetrics editorFontMetrics()
//    { return instance()->m_font_editor_metrics; }

    void load();
    bool update(const QString &path);
    QJsonObject get(const ScopePath *scope);

    struct Entry {
        ScopeSelector selector;
        QJsonObject options;

        // Used internally.
        ScopeScore score;
    };

    typedef QList<Entry> Options;
    Options options;

    struct Observer {
        ScopePath scope;
        QList<ConfigurationObserver*> targets;
        QJsonObject options;

        bool operator ==(const Observer &other);
    };

    Observer *find(const ScopePath &scope);
    Observer *find(const ScopePath &scope, ConfigurationObserver *o);

    typedef QList<Observer> Observers;
    Observers observers;

signals:
    void themeChanged(QSharedPointer<Theme> theme);
//    void editorFontChanged(QFont font, QFontMetrics metrics);
//    void uiFontChanged(QFont font, QFontMetrics metrics);

public slots:

private:
    QString m_path;
    QSharedPointer<Theme> m_theme;

//    QFont m_font_editor;
//    QFont m_font_ui;
//    QFontMetrics m_font_editor_metrics;
//    QFontMetrics m_font_ui_metrics;
};

#endif // CONFIGURATION_H
