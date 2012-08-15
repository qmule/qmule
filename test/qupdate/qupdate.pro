#-------------------------------------------------
#
# Project created by QtCreator 2012-08-07T21:31:35
#
#-------------------------------------------------

QT       += core gui network xml

TARGET = qupdate
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        ../../src/silent_updater.cpp \
    ../../src/taskbar_iface.cpp

HEADERS  += mainwindow.h \
    ../../src/silent_updater.h \
    ../../src/taskbar_iface.h

LIBS += ole32.lib user32.lib

FORMS    += mainwindow.ui

CONFIG(win7) {
    DEFINES += WIN7_SDK
}

RESOURCES += \
    icons.qrc
