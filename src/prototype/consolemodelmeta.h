#ifndef CONSOLEMODELMETA_H
#define CONSOLEMODELMETA_H

#include "modelmetaregistry.h"

class LuaEngine;

struct ConsoleModelMeta : public ModelMeta
{
    ConsoleModelMeta(LuaEngine *lua_engine);

    ModelView *createModelView(Panel *parent);
    Model *fromJson(const QJsonObject &j, QObject *parent);
    void toJson(Model *model, QJsonObject &j);

    LuaEngine *lua_engine;
};

#endif // CONSOLEMODELMETA_H
