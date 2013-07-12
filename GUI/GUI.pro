#-------------------------------------------------
#
# Project created by QtCreator 2013-06-02T01:22:04
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = C4Linux
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp\
        editorwindow.cpp \
    configdialog.cpp \
    c4fileiconprovider.cpp \
    filesortfilterproxymodel.cpp \
    inspectordialog.cpp \
    utilities.cpp

HEADERS  += editorwindow.h \
    configdialog.h \
    c4fileiconprovider.h \
    filesortfilterproxymodel.h \
    inspectordialog.h \
    utilities.h

FORMS    += editorwindow.ui \
		editorconfdiag.ui \
    editorinspectordiag.ui

RESOURCES += \
    icons.qrc

unix:!macx: LIBS += -L$$OUT_PWD/../MemFUSE/ -lMemFUSE

INCLUDEPATH += $$PWD/../MemFUSE
DEPENDPATH += $$PWD/../MemFUSE

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../MemFUSE/libMemFUSE.a

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += fuse
unix: PKGCONFIG += zlib
unix: PKGCONFIG += icu-uc

unix|win32: LIBS += -L$$OUT_PWD/../C4Group/ -lC4Group

INCLUDEPATH += $$PWD/../C4Group
DEPENDPATH += $$PWD/../C4Group

win32: PRE_TARGETDEPS += $$OUT_PWD/../C4Group/C4Group.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../C4Group/libC4Group.a
