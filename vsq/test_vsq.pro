#-------------------------------------------------
#
# Project created by QtCreator 2012-02-13T03:20:30
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = test_vsq
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    TimesigTest.cpp \
    main.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

INCLUDEPATH += ../../vsq/
LIBS += -lvsq -lcppunit -L"../../vsq/"

HEADERS += \
    Util.h
