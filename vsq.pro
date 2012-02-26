#-------------------------------------------------
#
# Project created by QtCreator 2012-02-13T03:14:38
#
#-------------------------------------------------

QT       -= core gui

TARGET = vsq
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    Timesig.cpp \
    TimesigList.cpp \
    MeasureLineIterator.cpp \
    MeasureLine.cpp

HEADERS += \
    Timesig.h \
    TimesigList.h \
    MeasureLineIterator.h \
    MeasureLine.h \
    vsqglobal.h \
    HandleType.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

INCLUDEPATH += ../ ./
