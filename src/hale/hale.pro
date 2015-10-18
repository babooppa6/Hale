QT += core gui widgets winextras # core-private gui-private

TARGET = hale
TEMPLATE = app

CONFIG += c++11
# CONFIG += console

DEFINES += "ONIG_EXTERN=extern"
DEFINES += __PROJECT__=\\\"$$PWD/\\\"
DEFINES += __WPROJECT__=L\\\"$$PWD/\\\"
#DEFINES += __uPROJECT__=\\\"$$PWD/\\\"
win32: RC_FILE = $${PWD}/app.rc

unix: {
    QMAKE_CXXF

LAGS_WARN_ON += -Wno-reorder

# TODO:
#APP_QML_FILES.files = path/to/file1.qml path/to/file2.qml
#APP_QML_FILES.path = Contents/Resources
#QMAKE_BUNDLE_DATA += APP_QML_FILES

#    QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
#    QMAKE_POST_LINK += mkdir -p $${DESTDIR}/Inua.app/Contents/Frameworks
#    QMAKE_POST_LINK += &
#    QMAKE_POST_LINK += cp $${OUT_PWD}/../Lua/liblua51.1.0.0.dylib $${DESTDIR}/Inua.app/Contents/Frameworks/liblua51.1.dylib
}

win32 {
    DESTDIR = $${PWD}/../data
    PRECOMPILED_HEADER = precompiled.h
}

#win32 {
#   # DESTDIR = $${PWD}/../../dist
#   # QMAKE_POST_LINK = copy /Y \"$$shell_path($${OUT_PWD}/debug/$${TARGET}.pdb)\" \"$$shell_path($${DESTDIR})\"
#}

HEADERS += \
    application.h \
    project.h \
    document.h \
    util.h \
    projectmanager.h \
    commandmanager.h \
    grammar.h \
    onigregexp.h \
    parser.h \
    tdocumentbuffer.h \
    documenttypes.h \
    theme.h \
    documentparser.h \
    mainwindow.h \
    precompiled.h \
    luaengine.h \
    luautil.h \
    luaobject.h \
    undostream.h \
    documenteditor.h \
    documenteditorlayout.h \
    shellpanel.h \
    pty.h \
    finderboyermoore.h \
    configuration.h \
    statusline.h \
    panel.h \
    model.h \
    scopepath.h \
    lua_scopepath.h \
    enums.h \
    lua_documentmodel.h \
    lua_consolemodel.h \
    consolemodel.h \
    tlua.h \
    tlua_table.h \
    tlua_value.h \
    tlua_reference.h \
    tlua_function.h \
    tlua_userdata.h \
    tlua_settings.h \
    lua_app.h \
    tlua_qt.h \
    tlua_lib.h \
    frameless.h \
    pathutil.h \
    pathcompleter.h \
    pathcompletioncontroller.h \
    lua_panel.h \
    tlua_registry.h \
    tlua_helpers.h \
    objectfactory.h \
    documentmodel.h \
    modelmetaregistry.h \
    documentmodelmeta.h \
    documentmodelview.h \
    consolemodelview.h \
    consolemodelmeta.h \
    switchwindow.h \
    switchcontroller.h \
    clipboard.h \
    completion.h \
    lua_document.h \
    windowheader.h \
    option.h \
    configurationobserver.h \
    scopestore.h \
    hale_types.h \
    hale_vector.h \
    hale_gap_buffer.h \
    hale.h \
    hale_document.h \
    hale_stream.h \
    hale_string.h \
    hale_platform.h \
    hale_test.h \
    hale_perf.h \
    hale_fixed_gap_buffer.h \
    hale_print.h \
    hale_util.h \
    hale_platform_win32.h \
    hale_encoding.h \
    hale_encoding_mib.h \
    hale_helper_file.h \
    hale_custom.h \
    hale_platform_win32_ui.h \
    hale_memory.h \
    hale_math.h \
    hale_platform_win32_gdi.h \
    hale_platform_win32_dx.h

SOURCES += \
    main.cpp \
    application.cpp \
    project.cpp \
    document.cpp \
    util.cpp \
    projectmanager.cpp \
    commandmanager.cpp \
    grammar.cpp \
    onigregexp.cpp \
    parser.cpp \
    documenttypes.cpp \
    theme.cpp \
    documentparser.cpp \
    mainwindow.cpp \
    luaengine.cpp \
    luaobject.cpp \
    undostream.cpp \
    documenteditor.cpp \
    documenteditorlayout.cpp \
    shellpanel.cpp \
    pty.cpp \
    finderboyermoore.cpp \
    configuration.cpp \
    statusline.cpp \
    panel.cpp \
    model.cpp \
    scopepath.cpp \
    lua_scopepath.cpp \
    lua_consolemodel.cpp \
    lua_documentmodel.cpp \
    consolemodel.cpp \
    luautil.cpp \
    tlua_settings.cpp \
    lua_app.cpp \
    frameless.cpp \
    pathutil.cpp \
    pathcompleter.cpp \
    pathcompletioncontroller.cpp \
    lua_panel.cpp \
    documentmodel.cpp \
    modelmetaregistry.cpp \
    documentmodelmeta.cpp \
    documentmodelview.cpp \
    consolemodelview.cpp \
    consolemodelmeta.cpp \
    switchwindow.cpp \
    switchcontroller.cpp \
    clipboard.cpp \
    completion.cpp \
    lua_document.cpp \
    windowheader.cpp \
    option.cpp \
    configurationobserver.cpp \
    scopestore.cpp \
    hale_document.cpp \
    hale_gap_buffer.cpp \
    hale_document_parse.cpp \
    hale_string.cpp \
    hale.cpp \
    hale_test.cpp \
    hale_test_document.cpp \
    hale_fixed_gap_buffer.cpp \
    hale_print.cpp \
    hale_util.cpp \
    hale_document_session.cpp \
    hale_document_arena.cpp \
    hale_platform_darwin.cpp \
    hale_platform_win32.cpp \
    hale_encoding.cpp \
    hale_test_encoding.cpp \
    hale_test_encoding_referential.cpp \
    hale_encoding_utf8.cpp \
    hale_encoding_hale.cpp \
    hale_encoding_utf16.cpp \
    hale_platform_win32_ui.cpp \
    hale_platform_win32_gdi.cpp \
    hale_platform_win32_dx.cpp

RESOURCES += \
    app.qrc

OTHER_FILES += \
    Notes.txt

#
# Oniguruma
#

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Oniguruma/release/ -lOniguruma
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Oniguruma/debug/ -lOniguruma
else:unix: LIBS += -L$$OUT_PWD/../Oniguruma/ -lOniguruma

INCLUDEPATH += $$PWD/../Oniguruma
DEPENDPATH += $$PWD/../Oniguruma

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/release/libOniguruma.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/debug/libOniguruma.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/release/Oniguruma.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/debug/Oniguruma.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/libOniguruma.a

#
# Lua
#

win32:CONFIG(release, debug|release): LIBS += -L$$DESTDIR/lib/ -llua51
else:win32:CONFIG(debug, debug|release): LIBS += -L$$DESTDIR/lib/ -llua51
# else:unix: LIBS += -L$$PWD/../Data/lib/ -llua51
else:unix: LIBS += -L$$OUT_PWD/../Lua/ -llua51

INCLUDEPATH += $$PWD/../Lua
DEPENDPATH += $$PWD/../Lua
