# QT -= core #gui widgets winextras # core-private gui-private

QT -= core

TARGET = hale
#TEMPLATE = app

CONFIG += c++11
# CONFIG += console

DEFINES += "HALE_INCLUDES=1"
DEFINES += "HALE_STU=1"
DEFINES += "ONIG_EXTERN=extern"
DEFINES += __PROJECT__=\\\"$$PWD/\\\"
DEFINES += __WPROJECT__=L\\\"$$PWD/\\\"
#DEFINES += __uPROJECT__=\\\"$$PWD/\\\"
# win32: RC_FILE = $${PWD}/app.rc

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
    # PRECOMPILED_HEADER = precompiled.h
}

#win32 {
#   # DESTDIR = $${PWD}/../../dist
#   # QMAKE_POST_LINK = copy /Y \"$$shell_path($${OUT_PWD}/debug/$${TARGET}.pdb)\" \"$$shell_path($${DESTDIR})\"
#}

# HEADERS += \


SOURCES += \
    stu.cpp

OTHER_FILES += \
    Notes.txt \
    hale_types.h \
    hale_vector.h \
    hale_gap_buffer.h \
    hale.h \
    hale_document.h \
    hale_stream.h \
    hale_string.h \
    hale_test.h \
    hale_perf.h \
    hale_fixed_gap_buffer.h \
    hale_print.h \
    hale_util.h \
    hale_encoding.h \
    hale_encoding_mib.h \
    hale_helper_file.h \
    hale_custom.h \
    hale_memory.h \
    hale_math.h \
    hale_ui.h \
    hale_os.h \
    hale_os_ui.h \
    hale_document_types.h \
    hale_document_view.h \
    hale_math_interpolation.h \
    jsmn.h \
    hale_document_parser.h \
    hale_parser_c.h \
    hale_atoi.h \
    os_win32.h \
    os_win32_dx.h \
    os_win32_gdi.h \
    os_win32_ui.h \
    hale_macros.h \
    hale_config.h \
    hale_document.cpp \
    hale_gap_buffer.cpp \
    hale_document_parse.cpp \
    hale_string.cpp \
    hale.cpp \
    hale_test.cpp \
    hale_fixed_gap_buffer.cpp \
    hale_print.cpp \
    hale_util.cpp \
    hale_document_arena.cpp \
    hale_encoding.cpp \
    hale_ui.cpp \
    hale_document_view.cpp \
    jsmn.cpp \
    hale_parser_c.cpp \
    hale_atoi.cpp \
    hale_os_ui.cpp \
    os_win32.cpp \
    os_win32_dx.cpp \
    os_win32_gdi.cpp \
    os_win32_ui.cpp \
    test_document.cpp \
    test_encoding.cpp \
    test_encoding_referential.cpp \
    os_darwin.cpp \
    _stu.cpp \
    _hale_encoding_hale.cpp \
    _hale_encoding_utf16.cpp \
    _hale_encoding_utf8.cpp \
    hale_os.cpp

#
# Oniguruma
#

#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Oniguruma/release/ -lOniguruma
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Oniguruma/debug/ -lOniguruma
#else:unix: LIBS += -L$$OUT_PWD/../Oniguruma/ -lOniguruma

#INCLUDEPATH += $$PWD/../Oniguruma
#DEPENDPATH += $$PWD/../Oniguruma

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/release/libOniguruma.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/debug/libOniguruma.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/release/Oniguruma.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/debug/Oniguruma.lib
#else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Oniguruma/libOniguruma.a

#
# Lua
#

#win32:CONFIG(release, debug|release): LIBS += -L$$DESTDIR/lib/ -llua51
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$DESTDIR/lib/ -llua51
## else:unix: LIBS += -L$$PWD/../Data/lib/ -llua51
#else:unix: LIBS += -L$$OUT_PWD/../Lua/ -llua51

#INCLUDEPATH += $$PWD/../Lua
#DEPENDPATH += $$PWD/../Lua
