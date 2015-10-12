#-------------------------------------------------
#
# Project created by QtCreator 2015-08-07T15:11:07
#
#-------------------------------------------------

QT       -= core gui

TARGET = lua51
TEMPLATE = lib
CONFIG += sharedlib

QMAKE_LFLAGS_SONAME  = -Wl,-install_name,$${OUT_PWD}/

DESTDIR = $${PWD}/../data/lib

unix: {
    DEFINES += LUA_USE_MACOSX
    # DEFINES += LUA_LIB
}

win32: {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += LUA_LIB
    DEFINES += LUA_BUILD_AS_DLL
    QMAKE_POST_LINK += mkdir \"$$shell_path($${DESTDIR}/../include)\" &
    QMAKE_POST_LINK += copy /Y \"$$shell_path($${PWD}/lauxlib.h)\" \"$$shell_path($${DESTDIR}/../include/*)\" &
    QMAKE_POST_LINK += copy /Y \"$$shell_path($${PWD}/lua.h)\" \"$$shell_path($${DESTDIR}/../include/*)\" &
    QMAKE_POST_LINK += copy /Y \"$$shell_path($${PWD}/lua.hpp)\" \"$$shell_path($${DESTDIR}/../include/*)\" &
    QMAKE_POST_LINK += copy /Y \"$$shell_path($${PWD}/luaconf.h)\" \"$$shell_path($${DESTDIR}/../include/*)\" &
    QMAKE_POST_LINK += copy /Y \"$$shell_path($${PWD}/lualib.h)\" \"$$shell_path($${DESTDIR}/../include/*)\" &
    # QMAKE_POST_LINK += copy /Y \"$$shell_path($${DESTDIR}/lua51.lib)\" \"$$shell_path($${DESTDIR}/lua5.1.lib)\" &
    QMAKE_POST_LINK += move /Y \"$$shell_path($${DESTDIR}/lua51.dll)\" \"$$shell_path($${DESTDIR}/../lua51.dll)\"
}

#unix: {
#    DESTDIR = $${PWD}/../Data/lib
#    QMAKE_POST_LINK += cp \"$$shell_path($${PWD}/lauxlib.h)\" \"$$shell_path($${DESTDIR}/../include)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${PWD}/lua.h)\" \"$$shell_path($${DESTDIR}/../include)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${PWD}/lua.hpp)\" \"$$shell_path($${DESTDIR}/../include)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${PWD}/luaconf.h)\" \"$$shell_path($${DESTDIR}/../include)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${PWD}/lualib.h)\" \"$$shell_path($${DESTDIR}/../include)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${DESTDIR}/lua51.lib)\" \"$$shell_path($${DESTDIR}/lua5.1.lib)\" &
#    QMAKE_POST_LINK += cp \"$$shell_path($${DESTDIR}/lua51.dll)\" \"$$shell_path($${DESTDIR}/lua5.1.dll)\"
#}

SOURCES += \
    lapi.c \
    lauxlib.c \
    lbaselib.c \
    lcode.c \
    ldblib.c \
    ldebug.c \
    ldo.c \
    ldump.c \
    lfunc.c \
    lgc.c \
    linit.c \
    liolib.c \
    llex.c \
    lmathlib.c \
    lmem.c \
    loadlib.c \
    lobject.c \
    lopcodes.c \
    loslib.c \
    lparser.c \
    lstate.c \
    lstring.c \
    lstrlib.c \
    ltable.c \
    ltablib.c \
    ltm.c \
    lua.c \
    lundump.c \
    lvm.c \
    lzio.c \
    print.c

HEADERS += \
    lapi.h \
    lauxlib.h \
    lcode.h \
    ldebug.h \
    ldo.h \
    lfunc.h \
    lgc.h \
    llex.h \
    llimits.h \
    lmem.h \
    lobject.h \
    lopcodes.h \
    lparser.h \
    lstate.h \
    lstring.h \
    ltable.h \
    ltm.h \
    lua.h \
    luaconf.h \
    lualib.h \
    lundump.h \
    lvm.h \
    lzio.h \
    lua.hpp
#unix {
#    target.path = DESTDIR
#    INSTALLS += target
#}
