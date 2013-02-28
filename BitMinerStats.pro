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
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

HEADERS += \
    graph.h

SOURCES += \
    graph.cpp
