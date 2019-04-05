TEMPLATE = lib
CONFIG += plugin
QT += widgets thelib
TARGET = tsprint

LIBS += -lcups

INCLUDEPATH += ../../shell/statuscenter/

HEADERS += \
    plugin.h \
    printermanagement.h

SOURCES += \
    plugin.cpp \
    printermanagement.cpp

unix {
    blueprint {
        target.path = /usr/lib/theshellb/panes/
    } else {
        target.path = /usr/lib/theshell/panes/
    }

    INSTALLS += target
}

FORMS += \
    printermanagement.ui

DISTFILES += \
    metadata.json
