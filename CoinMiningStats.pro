#-------------------------------------------------
#
# Project created by QtCreator 2013-02-27T10:38:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CoinMiningStats
TEMPLATE = app

linux-g++ {
    QT += dbus
    DEFINES += DBUS_NOTIFICATIONS
}

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/graph.cpp \
    src/settings.cpp \
    src/loosejson.cpp \
    src/manageminers.cpp \
    src/colorindicatorlabel.cpp \
    src/miner.cpp

HEADERS  += src/mainwindow.h \
    src/graph.h \
    src/loosejson.h \
    src/manageminers.h \
    src/colorindicatorlabel.h \
    src/settings.h \
    src/miner.h

FORMS    += src/mainwindow.ui \
    src/settings.ui \
    src/manageminers.ui

OTHER_FILES += \
    widget.css

RESOURCES += \
    resources.qrc
