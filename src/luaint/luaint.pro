TARGET = lua
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $${PWD}/../data

unix: {
    DEFINES += LUA_USE_MACOSX
}

win32 {
    DEFINES += LUA_LIB
    DEFINES += LUA_BUILD_AS_DLL
    # DESTDIR = $${PWD}/../../dist
    # QMAKE_POST_LINK = copy /Y \"$$shell_path($${OUT_PWD}/debug/$${TARGET}.pdb)\" \"$$shell_path($${DESTDIR})\"
}

SOURCES += main.c

include(deployment.pri)
qtcAddDeployment()


#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lua/release/ -lLua
##else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lua/debug/ -lLua
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../data/lib/ -llua51
#else:unix: LIBS += -L$$OUT_PWD/../lua/ -lLua

#INCLUDEPATH += $$PWD/../Lua
#DEPENDPATH += $$PWD/../Lua

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua/release/libLua.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua/debug/libLua.a
##else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua/release/Lua.lib
##else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lua/debug/Lua.lib
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../data/lib/lua51.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../data/lib/lua51.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../data/lib/lua51.lib

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../data/lib/ -llua51
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../data/lib/ -llua51
else:unix: LIBS += -L$$PWD/../data/lib -llua51

INCLUDEPATH += $$PWD/../lua
DEPENDPATH += $$PWD/../lua
