#-------------------------------------------------
#
# Project created by QtCreator 2016-05-01T16:23:34
#
#-------------------------------------------------

QT       += core gui dbus thelib x11extras
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += x11 polkit-qt5-1 polkit-qt5-agent-1
}

blueprint {
    TARGET = ts-polkitagentb
    SHARE_APP_NAME=theshell/ts-polkitagentb

    DEFINES += "BLUEPRINT"
} else {
    TARGET = ts-polkitagent
    SHARE_APP_NAME = theshell/ts-polkitagent
}

TEMPLATE = app
DBUS_ADAPTORS = org.thesuite.polkitAuthAgent.xml

SOURCES += main.cpp\
    polkitinterface.cpp \
    authenticate.cpp

HEADERS  += \
    polkitinterface.h \
    authenticate.h

FORMS    += \
    authenticate.ui

include(/usr/share/the-libs/pri/buildmaster.pri)

unix {
    target.path = /usr/lib

    INSTALLS += target
}
