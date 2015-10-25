#ifndef DOCUMENTMODELMETA_H
#define DOCUMENTMODELMETA_H

#include "modelmetaregistry.h"
class DocumentManager;

struct DocumentModelMeta : public ModelMeta
{
    DocumentModelMeta(DocumentManager *document_manager);

    ModelView *createModelView(Panel *parent);
    Model *fromJson(const QJsonObject &j, QObject *parent);
    void toJson(Model *model, QJsonObject &j);

    DocumentManager *document_manager;
};

#endif // DOCUMENTMODELMETA_H
