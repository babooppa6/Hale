#include <QUuid>

#include "document.h"

#include "documentmodel.h"
#include "documentmodelview.h"
#include "documentmodelmeta.h"

DocumentModelMeta::DocumentModelMeta(DocumentManager *dm) :
    document_manager(dm)
{}

ModelView *DocumentModelMeta::createModelView(Panel *parent)
{
    return new DocumentModelView(parent);
}

Model *DocumentModelMeta::fromJson(const QJsonObject &j, QObject *parent)
{
    Q_ASSERT(document_manager);

    QUuid document_id(j["document"].toString());
    Q_ASSERT(!document_id.isNull());

    auto document = document_manager->loadDocument(document_id);
    if (!document) {
        return NULL;
    }

    auto dm = new DocumentModel(document, parent);
    dm->fromJson(j);

    return dm;
}

void DocumentModelMeta::toJson(Model *model, QJsonObject &j)
{
    DocumentModel *dm = static_cast<DocumentModel*>(model);
    j["document"] = dm->document()->id().toString();
}
