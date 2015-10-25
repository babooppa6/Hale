#include "model.h"
#include "modelmetaregistry.h"

ModelMeta::ModelMeta() {}

Model *ModelMetaRegistry::fromJson(const QJsonObject &j, QObject *parent)
{
    auto meta = metaFor(j["type"].toString());
    Q_ASSERT(meta);
    return meta->fromJson(j, parent);
}

void ModelMetaRegistry::toJson(Model *model, QJsonObject &j)
{
    j["type"] = model->metaObject()->className();
    auto meta = metaFor(model);
    Q_ASSERT(meta);
    meta->toJson(model, j);
}

ModelMeta *ModelMetaRegistry::metaFor(Model *model)
{
    return metaFor(model->metaObject()->className());
}

ModelMeta *ModelMetaRegistry::metaFor(const char *class_name)
{
    return metaFor(QByteArray(class_name));
}

ModelMeta *ModelMetaRegistry::metaFor(const QByteArray &class_name)
{
    Map::iterator it = map().find(class_name);
    if (it == map().end()) {
        qWarning() << "Meta object for class" << class_name << "not found.";
        Q_ASSERT(0 && "Meta object for class not found.");
        return NULL;
    }
    return it.value();
}

ModelMeta *ModelMetaRegistry::metaFor(const QString &class_name)
{
    return metaFor(class_name.toLatin1());
}
