#include "consolemodel.h"
#include "consolemodelview.h"
#include "consolemodelmeta.h"

ConsoleModelMeta::ConsoleModelMeta(LuaEngine *lua_engine) :
    lua_engine(lua_engine)
{
}

ModelView *ConsoleModelMeta::createModelView(Panel *parent)
{
    return new ConsoleModelView(parent);
}

Model *ConsoleModelMeta::fromJson(const QJsonObject &j, QObject *parent)
{
    Q_UNUSED(j);
    Q_ASSERT(lua_engine);
    return new ConsoleModel(parent, lua_engine);
}

void ConsoleModelMeta::toJson(Model *, QJsonObject &)
{
}
