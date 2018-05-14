#-------------------------------------------------
#
# Project created by QtCreator 2016-05-01T16:23:34
#
#-------------------------------------------------

QT       += core gui dbus thelib x11extras
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
LIBS += -lpolkit-qt5-agent-1 -lpolkit-qt5-core-1 -lX11

blueprint {
    TARGET = ts-polkitagentb

    DEFINES += "BLUEPRINT"
} else {
    TARGET = ts-polkitagent
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

unix {
    target.path = /usr/lib

    INSTALLS += target
}
