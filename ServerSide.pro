#-------------------------------------------------
#
# Project created by QtCreator 2019-01-12T00:53:59
#
#-------------------------------------------------

QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OSSIPlayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    downloadmanager.cpp \
    settings.cpp \
    orderitem.cpp \
    playdate.cpp

HEADERS  += mainwindow.h \
    downloadmanager.h \
    settings.h \
    orderitem.h \
    playdate.h

FORMS    += mainwindow.ui \
    settings.ui

LIBS += -lKernel32 -luser32 -lshell32 -lgdi32

LIBS       += -L"$$PWD\vlc-qt\x64\debug\lib" -lVLCQtCore -lVLCQtWidgets

INCLUDEPATH += "$$PWD\vlc-qt\x64\include"
