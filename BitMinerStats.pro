#-------------------------------------------------
#
# Project created by QtCreator 2013-02-27T10:38:26
#
#-------------------------------------------------

QT       += core gui script network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BitMinerStats
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graph.cpp

HEADERS  += mainwindow.h \
    graph.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    widget.css

RESOURCES += \
    resources.qrc
