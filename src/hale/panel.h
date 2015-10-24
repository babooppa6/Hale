#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QUuid>
#include "luaobject.h"
#include "configurationobserver.h"

struct lua_State;

class QVBoxLayout;

class StatusLine;
class StatusLineController;

class Model;
class ModelView;
class ScopePath;

class Panel : public QWidget, public LuaObject, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit Panel(QWidget *parent = 0);
    ~Panel();

    QUuid id;

    int lua_wrap(lua_State *L);
    static Panel *lua_read(lua_State *L, int n);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setTitle(const QString &title);
    QString title()
    { return m_title; }

    StatusLine *statusLine()
    { return m_status_line; }

    void scope(ScopePath *o_path);
    void configure(QJsonObject &object);

    // void scope(ScopePath *o_path, Model *model);

    Model *model()
    { return current_model; }
    void setModel(Model *model);
    void peekModel(Model *model);
    void addModel(Model *model);
    Model *removeModel(Model *model);
    bool hasModel(Model *model);

    // This is called from panel manager when this views becomes active.
    void setActive(bool active);

    void toJson(QJsonObject &json, void *context);
    void fromJson(const QJsonObject &json, void *context);

    typedef QList<Model *> Models;
    Models models;
    Model *current_model;

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
//    void focusInEvent(QFocusEvent *);
//    void focusOutEvent(QFocusEvent *);

signals:
    void modelChanged(Model *old_model, Model *new_model);
    void titleChanged();

private:
    QString m_title;
    bool m_active;
    StatusLine *m_status_line;
    QVBoxLayout *m_layout;

    typedef QList<ModelView *> Views;
    Views m_views;
    ModelView *m_view;

    void setView(ModelView *view, Model *model);
    ModelView *getView(Model *model);

    void layoutView();

    bool hasStatusLine() const;
};

LUAOBJECT_DECLARE(Panel)

//
//
//

class ModelView : public QWidget
{
    Q_OBJECT
public:
    ModelView(Panel *panel);

    virtual void scopePath(ScopePath *, Model *) {}
    virtual void addModel(Model *) {}
    virtual void removeModel(Model *) {}
    virtual void setModel(Model *) = 0;
    virtual bool supportsModel(Model *) = 0;
};

//
//
//

class DefaultModelView : public ModelView
{
    Q_OBJECT
public:
    DefaultModelView(Panel *panel);

//    QSize sizeHint() const;
    virtual void setModel(Model *);
    virtual bool supportsModel(Model *);

protected:
    void paintEvent(QPaintEvent *);
};

#endif // PANEL_H
