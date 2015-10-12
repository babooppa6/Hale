#-------------------------------------------------
#
# Project created by QtCreator 2014-12-28T02:41:48
#
#-------------------------------------------------

QT       -= core gui



DEFINES += "ONIG_EXTERN=extern"
DEFINES += _CRT_SECURE_NO_WARNINGS

TARGET = Oniguruma
TEMPLATE = lib
CONFIG += staticlib

win32: QMAKE_CXXFLAGS += /wd4244 # conversion from '__int64' to 'int', possible loss of data
win32: QMAKE_CFLAGS += /wd4244 # conversion from '__int64' to 'int', possible loss of data
# QMAKE_CXXFLAGS += /wd4273 # inconsistent DLL linkage

SOURCES += \
    regcomp.c \
    regenc.c \
    regerror.c \
    regexec.c \
    regext.c \
    reggnu.c \
    regparse.c \
    regposerr.c \
    regposix.c \
    regsyntax.c \
    regtrav.c \
    regversion.c \
    st.c \
    enc/ascii.c \
    enc/big5.c \
    enc/cp1251.c \
    enc/euc_jp.c \
    enc/euc_kr.c \
    enc/euc_tw.c \
    enc/gb18030.c \
    enc/iso8859_1.c \
    enc/iso8859_2.c \
    enc/iso8859_3.c \
    enc/iso8859_4.c \
    enc/iso8859_5.c \
    enc/iso8859_6.c \
    enc/iso8859_7.c \
    enc/iso8859_8.c \
    enc/iso8859_9.c \
    enc/iso8859_10.c \
    enc/iso8859_11.c \
    enc/iso8859_13.c \
    enc/iso8859_14.c \
    enc/iso8859_15.c \
    enc/iso8859_16.c \
    enc/koi8.c \
    enc/koi8_r.c \
    enc/mktable.c \
    enc/sjis.c \
    enc/unicode.c \
    enc/utf8.c \
    enc/utf16_be.c \
    enc/utf16_le.c \
    enc/utf32_be.c \
    enc/utf32_le.c

HEADERS += oniguruma.h \
    config.h \
    oniggnu.h \
    onigposix.h \
    regenc.h \
    regint.h \
    regparse.h \
    st.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/
