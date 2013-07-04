#-------------------------------------------------
#
# Project created by QtCreator 2013-06-30T18:45:04
#
#-------------------------------------------------

QT       -= core gui

TARGET = MemFUSE
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    memfs.cpp \
    fuseWrap.cpp

HEADERS += \
    memfs.h \
    fuseWrap.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += fuse
