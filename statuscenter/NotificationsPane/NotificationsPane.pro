#-------------------------------------------------
#
# Project created by QtCreator 2019-01-16T23:00:00
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib positioning network multimedia

TARGET = tsnotifications
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

blueprint {
    DEFINES += "BLUEPRINT"
    LIBS += -ltheshell-libb

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/notificationpane/\\\""
} else {
    LIBS += -ltheshell-lib
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/notificationpane/\\\""
}

DBUS_ADAPTORS += org.kde.JobViewV2.xml

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
        translations.path = /usr/share/theshellb/notificationpane/translations
    } else {
        target.path = /usr/lib/theshell/panes/
        translations.path = /usr/share/theshell/notificationpane/translations
    }
    INSTALLS += target translations
}

DISTFILES += \
    NotificationPane.json \
    org.kde.JobViewV2.xml

HEADERS += \
    plugin.h \
    audiomanager.h \
    kjob/jobdbus.h \
    kjob/jobserver.h \
    kjob/jobviewserver_adaptor.h \
    notificationsWidget/mediaplayernotification.h \
    notificationsWidget/notificationappgroup.h \
    notificationsWidget/notificationobject.h \
    notificationsWidget/notificationpanel.h \
    notificationsWidget/notificationpopup.h \
    notificationsWidget/notificationsdbusadaptor.h \
    notificationsWidget/notificationswidget.h \
    kjob/jobviewwidget.h \
    settings/settingspane.h \
    settings/applicationnotificationmodel.h

SOURCES += \
    plugin.cpp \
    audiomanager.cpp \
    kjob/jobdbus.cpp \
    kjob/jobserver.cpp \
    kjob/jobviewserver_adaptor.cpp \
    notificationsWidget/mediaplayernotification.cpp \
    notificationsWidget/notificationappgroup.cpp \
    notificationsWidget/notificationobject.cpp \
    notificationsWidget/notificationpanel.cpp \
    notificationsWidget/notificationpopup.cpp \
    notificationsWidget/notificationsdbusadaptor.cpp \
    notificationsWidget/notificationswidget.cpp \
    kjob/jobviewwidget.cpp \
    settings/settingspane.cpp \
    settings/applicationnotificationmodel.cpp

FORMS += \
    notificationsWidget/notificationswidget.ui \
    notificationsWidget/notificationpopup.ui \
    notificationsWidget/notificationappgroup.ui \
    notificationsWidget/notificationpanel.ui \
    notificationsWidget/mediaplayernotification.ui \
    kjob/jobviewwidget.ui \
    settings/settingspane.ui

