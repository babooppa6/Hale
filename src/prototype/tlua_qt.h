#ifndef TLUA_QT_H
#define TLUA_QT_H

#include <QString>
#include "tlua_value.h"

namespace tlua {

DEFINE_TYPE_TEMPLATE_FOR(QString,      , return QString(lua_tostring(L, n)),           lua_pushstring(L, v.toUtf8().constData()); return 1);
DEFINE_TYPE_TEMPLATE_FOR(QByteArray,   , return QByteArray(lua_tostring(L, n)),           lua_pushstring(L, v.constData()); return 1);

}

#endif // TLUA_QT_H

