#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <QObject>

class ObjectFactory
{
public:
    template<class ClassT>
    static ClassT *createObject(const QString &class_name)
    {
        Constructor constructor = map().value(class_name.toUtf8());
        if (constructor == NULL) {
            qWarning() << "Unable to create object for" << class_name << ". Type was not registered.";
            return NULL;
        }
        ClassT *object = static_cast<ClassT*>((*constructor)());
        Q_ASSERT(object);
        if (object == NULL) {
            qCritical() << "Constructor returned NULL for" << class_name << ".";
            return NULL;
        }
        return object;
    }

    template<class ClassT>
    static void toJson(ClassT *object, QJsonObject &json, void *context)
    {
        if (map().find(object->metaObject()->className()) == map().end()) {
            qCritical() << "Class" << object->metaObject()->className() << "is not registered. The reader won't be able to read it.";
            return;
        }
        json["type"] = object->metaObject()->className();
        object->toJson(json, context);
//        QMetaObject::invokeMethod(object, "toJson", Qt::DirectConnection,
//                                  QGenericReturnArgument(),
//                                  Q_ARG(QJsonObject &, json)
//                                  );
    }

    template<class ClassT, class ParamT = void*>
    static ClassT *fromJson(const QJsonObject &json, ParamT context)
    {
        ClassT *object = createObject<ClassT>(json["type"].toString());
        if (object) {
            object->fromJson(json, context);
        }
//        QMetaObject::invokeMethod(object, "fromJson", Qt::DirectConnection,
//                                  QGenericReturnArgument(),
//                                  Q_ARG(const QJsonObject &, json));
        return object;
    }

    template<class ClassT>
    static void registerType()
    {
        map().insert(ClassT::staticMetaObject.className(), &constructor<ClassT>);
    }

    template<class ClassT>
    static QByteArray className()
    {
        Map::iterator it = map().find(ClassT::staticMetaObject.className());
        return it != map().end() ? it.key() : QByteArray();
    }

private:
    typedef QObject * (*Constructor)();

    template<class ClassT>
    static QObject *constructor()
    {
        return new ClassT();
    }

    typedef QMap<QByteArray, Constructor> Map;
    static Map &map()
    {
        static Map map;
        return map;
    }
};

#endif // OBJECTFACTORY_H
