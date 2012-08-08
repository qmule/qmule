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
        ../../src/silent_updater.cpp

HEADERS  += mainwindow.h \
    ../../src/silent_updater.h

FORMS    += mainwindow.ui
