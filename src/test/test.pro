#-------------------------------------------------
#
# Project created by QtCreator 2015-01-08T02:02:31
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = test
CONFIG   += console
CONFIG   -= app_bundle

CONFIG += c++11

TEMPLATE = app


SOURCES += \
    stringdistancetest.cpp \
    main.cpp \
    pathutiltest.cpp \
    scopeselectortest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    stringdistancetest.h \
    pathutiltest.h \
    scopeselectortest.h

INCLUDEPATH += $$PWD/../hale
INCLUDEPATH += $$PWD/../lua
INCLUDEPATH += $$PWD/../oniguruma
