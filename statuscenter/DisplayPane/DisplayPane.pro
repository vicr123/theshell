#-------------------------------------------------
#
# Project created by QtCreator 2019-03-26T23:10:56
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib svg x11extras network

TARGET = tsdisplay
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += x11 xrandr
}

blueprint {
    DEFINES += "BLUEPRINT"

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/display/\\\""
} else {
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/display/\\\""
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


# Include the-libs build tools
include(/usr/share/the-libs/pri/gentranslations.pri)


unix {
    translations.files = translations/*.qm

    blueprint {
        target.path = /usr/lib/theshellb/panes/
        translations.path = /usr/share/theshellb/display/translations
    } else {
        target.path = /usr/lib/theshell/panes/
        translations.path = /usr/share/theshell/display/translations
    }
    INSTALLS += target translations
}

DISTFILES += \
    DisplayPane.json

HEADERS += \
    plugin.h \
    displaypane.h \
    displaypositionwidget.h \
    displayarrangementwidget.h \
    displayconfigurationwidget.h \
    redshiftengine.h \
    nativeeventfilter.h

SOURCES += \
    plugin.cpp \
    displaypane.cpp \
    displaypositionwidget.cpp \
    displayarrangementwidget.cpp \
    displayconfigurationwidget.cpp \
    redshiftengine.cpp \
    nativeeventfilter.cpp

FORMS += \
    displaypane.ui \
    displaypositionwidget.ui \
    displayarrangementwidget.ui \
    displayconfigurationwidget.ui
