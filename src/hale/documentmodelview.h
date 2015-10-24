#ifndef DOCUMENTPANEL_H
#define DOCUMENTPANEL_H

#include "panel.h"

class DocumentEditor;

class DocumentModelView : public ModelView
{
    Q_OBJECT
public:
    DocumentModelView(Panel *panel);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    virtual void scopePath(ScopePath *, Model *model);

    virtual bool supportsModel(Model *model);

    virtual void addModel(Model *model);
    virtual void removeModel(Model *model);
    virtual void setModel(Model *model);

private:
    DocumentEditor *m_editor;
};

#endif // DOCUMENTPANEL_H
