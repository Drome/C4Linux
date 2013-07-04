#-------------------------------------------------
#
# Project created by QtCreator 2013-06-30T18:57:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = C4Group
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CFLAGS += -std=c99

SOURCES += \
    c4group.c

HEADERS += \
    c4group.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
