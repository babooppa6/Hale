#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class Model;

class Panel;
class ModelView;

struct ModelMeta
{
    ModelMeta();

    virtual ModelView *createModelView(Panel *parent) = 0;
    virtual Model *fromJson(const QJsonObject &j, QObject *parent) = 0;
    virtual void toJson(Model *model, QJsonObject &j) = 0;
};

class ModelMetaRegistry
{
public:
    template<class T>
    static void addMeta(ModelMeta *meta) {
        map().insert(T::staticMetaObject.className(), meta);
    }

    /// Reads the model from JSON.
    static Model *fromJson(const QJsonObject &j, QObject *parent);
    /// Writes the model to JSON.
    static void toJson(Model *model, QJsonObject &j);

    /// Returns Meta object for given Model.
    static ModelMeta *metaFor(Model *model);
    /// Returns Meta object for given class.
    static ModelMeta *metaFor(const char *class_name);
    static ModelMeta *metaFor(const QByteArray &class_name);
    static ModelMeta *metaFor(const QString &class_name);

    /// Returns Meta object for given type.
    template<class T>
    static ModelMeta *metaFor() {
        return metaFor(T::staticMetaObject.className());
    }

private:
    typedef QMap<QByteArray, ModelMeta*> Map;

    static Map &map() {
        static Map map;
        return map;
    }
};

#endif // MODELFACTORY_H
