#-------------------------------------------------
#
# Project created by QtCreator 2013-02-27T10:38:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BitMinerStats
TEMPLATE = app

linux-g++ {
    QT += dbus
    DEFINES += DBUS_NOTIFICATIONS
}

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

HEADERS += \
    colorindicatorlabel.h

SOURCES += \
    colorindicatorlabel.cpp

HEADERS += \
    manageminers.h

SOURCES += \
    manageminers.cpp

FORMS += \
    manageminers.ui

HEADERS += \
    loosejson.h

SOURCES += \
    loosejson.cpp

HEADERS += \
    settings.h

SOURCES += \
    settings.cpp

FORMS += \
    settings.ui
