#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T17:22:06
#
#-------------------------------------------------

QT       += core gui dbus multimedia xml network positioning svg charts concurrent tdesktopenvironment
CONFIG   += c++14
LIBS     += -lcrypt -L$$OUT_PWD/../theshell-lib/

INCLUDEPATH += $$PWD/../theshell-lib
DEPENDPATH += $$PWD/../theshell-lib

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 x11 x11-xcb xcb-keysyms xscrnsaver xext libpulse libpulse-mainloop-glib libsystemd libunwind polkit-qt5-1 xi
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += TS_VERSION='\\"8.1\\"'

unix {
    QT += thelib x11extras
}

blueprint {
    TARGET = theshellb
    DEFINES += "BLUEPRINT"
    LIBS += -ltheshell-libb

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/\\\""
} else {
    TARGET = theshell
    LIBS += -ltheshell-lib

    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/\\\""
}

TEMPLATE = app
QDBUSXML2CPP_ADAPTOR_HEADER_FLAGS = -l NotificationDBus -i notificationdbus.h
QDBUSXML2CPP_ADAPTOR_SOURCE_FLAGS = -l NotificationDBus -i notificationdbus.h
#DBUS_ADAPTORS += org.freedesktop.Notifications.xml

#notifications.files = org.freedesktop.Notifications.xml
#notifications.header_flags = -l NotificationDBus -i notificationdbus.h
#DBUS_ADAPTORS += notifications

#DBUS_ADAPTORS += com.canonical.AppMenu.Registrar.xml

power.files = org.thesuite.power.xml

location.files = org.freedesktop.GeoClue2.Agent.xml
location.header_flags = -l LocationServices -i location/locationservices.h
location.source_flags = -l LocationServices -i location/locationservices.h

dsig.files = org.thesuite.theshell.xml

DBUS_ADAPTORS += power dsig

SOURCES += main.cpp\
        mainwindow.cpp \
    taskbarbutton.cpp \
    window.cpp \
    menu.cpp \
    endsessionwait.cpp \
    background.cpp \
    upowerdbus.cpp \
    infopanedropdown.cpp \
    clickablelabel.cpp \
    hoverframe.cpp \
    choosebackground.cpp \
    switch.cpp \
    FlowLayout/flowlayout.cpp \
    segfaultdialog.cpp \
    globalfilter.cpp \
    systrayicons.cpp \
    nativeeventfilter.cpp \
    internationalisation.cpp \
    dbusevents.cpp \
    rundialog.cpp \
    mousescrollwidget.cpp \
    onboarding.cpp \
    newmedia.cpp \
    bthandsfree.cpp \
    tutorialwindow.cpp \
    screenshotwindow.cpp \
    audiomanager.cpp \
    taskbarmanager.cpp \
    dbussignals.cpp \
    apps/appslistmodel.cpp \
    screenrecorder.cpp \
    location/locationservices.cpp \
    location/locationrequestdialog.cpp \
    agent_adaptor.cpp \
    locktypes/mousepassword.cpp \
    notificationsdbusadaptor.cpp

HEADERS  += mainwindow.h \
    taskbarbutton.h \
    window.h \
    menu.h \
    endsessionwait.h \
    background.h \
    upowerdbus.h \
    infopanedropdown.h \
    clickablelabel.h \
    hoverframe.h \
    choosebackground.h \
    switch.h \
    FlowLayout/flowlayout.h \
    segfaultdialog.h \
    globalfilter.h \
    systrayicons.h \
    nativeeventfilter.h \
    dbusevents.h \
    rundialog.h \
    mousescrollwidget.h \
    onboarding.h \
    newmedia.h \
    bthandsfree.h \
    tutorialwindow.h \
    screenshotwindow.h \
    audiomanager.h \
    internationalisation.h \
    taskbarmanager.h \
    dbussignals.h \
    apps/appslistmodel.h \
    screenrecorder.h \
    location/locationservices.h \
    location/locationrequestdialog.h \
    agent_adaptor.h \
    locktypes/mousepassword.h \
    statuscenter/statuscenterpane.h \
    statuscenter/statuscenterpaneobject.h \
    notificationsdbusadaptor.h

FORMS    += mainwindow.ui \
    menu.ui \
    endsessionwait.ui \
    background.ui \
    infopanedropdown.ui \
    choosebackground.ui \
    segfaultdialog.ui \
    rundialog.ui \
    onboarding.ui \
    newmedia.ui \
    tutorialwindow.ui \
    screenshotwindow.ui \
    location/locationrequestdialog.ui \
    locktypes/mousepassword.ui

DISTFILES += \
    org.freedesktop.Notifications.xml \
    theshell.desktop \
    theshellb.desktop \
    org.freedesktop.GeoClue2.Agent.xml \
    polkit/org.thesuite.theshell.policy

RESOURCES += \
    resources.qrc \
    resources2.qrc

TRANSLATIONS += translations/vi_VN.ts \
    translations/da_DK.ts \
    translations/es_ES.ts \
    translations/lt_LT.ts \
    translations/nl_NL.ts \
    translations/pl_PL.ts \
    translations/pt_BR.ts \
    translations/ru_RU.ts \
    translations/sv_SE.ts \
    translations/en_AU.ts \
    translations/en_US.ts \
    translations/en_GB.ts \
    translations/en_NZ.ts \
    translations/de_DE.ts \
    translations/id_ID.ts \
    translations/au_AU.ts \
    translations/it_IT.ts \
    translations/nb_NO.ts \
    translations/no_NO.ts \
    translations/ro_RO.ts \
    translations/cy_GB.ts \
    translations/fr_FR.ts

qtPrepareTool(LUPDATE, lupdate)
genlang.commands = "$$LUPDATE -no-obsolete -source-language en_US $$_PRO_FILE_"

qtPrepareTool(LRELEASE, lrelease)
rellang.commands = "$$LRELEASE -removeidentical $$_PRO_FILE_"
QMAKE_EXTRA_TARGETS = genlang rellang
PRE_TARGETDEPS = genlang rellang

unix {
    target.path = /usr/bin/

    translations.files = translations/*.qm
    xsession.path = /usr/share/xsessions

    headers.files = statuscenter/statuscenterpane.h statuscenter/statuscenterpaneobject.h
    ringtones.files = tones/*

    blueprint {
        translations.path = /usr/share/theshellb/translations
        xsession.files = theshellb.desktop
        ringtones.path = /usr/share/sounds/theshellb/tones
        headers.path = /usr/include/theshellb
    } else {
        translations.path = /usr/share/theshell/translations
        xsession.files = theshell.desktop
        ringtones.path = /usr/share/sounds/theshell/tones
        headers.path = /usr/include/theshell

        #Install polkit files only on theShell stable
        polkit.path = /usr/share/polkit-1/actions
        polkit.files = polkit/org.thesuite.theshell.policy
        INSTALLS += polkit
    }

    INSTALLS += target translations xsession ringtones headers
}
