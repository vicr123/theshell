#-------------------------------------------------
#
# Project created by QtCreator 2019-03-17T21:40:30
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib ModemManagerQt NetworkManagerQt network

TARGET = tsnm
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

blueprint {
    DEFINES += "BLUEPRINT"

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/tsnm/\\\""
} else {
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/tsnm/\\\""
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
        translations.path = /usr/share/theshellb/tsnm/translations
    } else {
        target.path = /usr/lib/theshell/panes/
        translations.path = /usr/share/theshell/tsnm/translations
    }
    INSTALLS += target translations
}

HEADERS += \
    networkmanager/connectioneditor/connectioneditor.h \
    networkmanager/connectioneditor/devicesettings.h \
    networkmanager/connectioneditor/devicesettingsmodel.h \
    networkmanager/connectioneditor/panes/gsmsettingspane.h \
    networkmanager/connectioneditor/panes/settingpane.h \
    networkmanager/connectioneditor/panes/simpinpane.h \
    networkmanager/connectioneditor/panes/wifisettingspane.h \
    networkmanager/devicepanel.h \
    networkmanager/enums.h \
    networkmanager/popovers/simpinrequest.h \
    plugin.h \
    networkmanager/availablenetworkslist.h \
    networkmanager/networkwidget.h \
    networkmanager/savednetworkslist.h \
    networkmanager/chunkwidget.h \
    networkmanager/securityinformationwidget.h \
    networkmanager/editwidget.h

SOURCES += \
    networkmanager/connectioneditor/connectioneditor.cpp \
    networkmanager/connectioneditor/devicesettings.cpp \
    networkmanager/connectioneditor/devicesettingsmodel.cpp \
    networkmanager/connectioneditor/panes/gsmsettingspane.cpp \
    networkmanager/connectioneditor/panes/settingpane.cpp \
    networkmanager/connectioneditor/panes/simpinpane.cpp \
    networkmanager/connectioneditor/panes/wifisettingspane.cpp \
    networkmanager/devicepanel.cpp \
    networkmanager/popovers/simpinrequest.cpp \
    plugin.cpp \
    networkmanager/availablenetworkslist.cpp \
    networkmanager/networkwidget.cpp \
    networkmanager/savednetworkslist.cpp \
    networkmanager/chunkwidget.cpp \
    networkmanager/securityinformationwidget.cpp \
    networkmanager/editwidget.cpp

DISTFILES += \
    NetworkPane.json

FORMS += \
    networkmanager/connectioneditor/connectioneditor.ui \
    networkmanager/connectioneditor/devicesettings.ui \
    networkmanager/connectioneditor/panes/gsmsettingspane.ui \
    networkmanager/connectioneditor/panes/simpinpane.ui \
    networkmanager/connectioneditor/panes/wifisettingspane.ui \
    networkmanager/devicepanel.ui \
    networkmanager/networkwidget.ui \
    networkmanager/chunkwidget.ui \
    networkmanager/popovers/simpinrequest.ui \
    networkmanager/securityinformationwidget.ui \
    networkmanager/editwidget.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += ModemManager libnm
