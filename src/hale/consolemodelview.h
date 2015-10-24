#ifndef CONSOLEPANEL_H
#define CONSOLEPANEL_H

#include "model.h"
#include "panel.h"

class Document;
class DocumentModel;
class DocumentEditor;
class ConsoleModel;

class LuaEngine;

class ConsoleModelView : public ModelView
{
    Q_OBJECT
public:
    explicit ConsoleModelView(Panel *parent);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    bool supportsModel(Model *model);
    void addModel(Model *model);
    void removeModel(Model *model);
    void setModel(Model *model);
    void scopePath(ScopePath *o_path, Model *model);

    // We have layout which will provide these.
    // QSize sizeHint() const;

signals:

public slots:

private:
    Model *m_model;
    DocumentEditor *m_log_editor;
    DocumentEditor *m_command_editor;
};

#endif // CONSOLEPANEL_H
