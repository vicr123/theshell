#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T17:22:06
#
#-------------------------------------------------

QT       += core gui gui-private x11extras dbus multimedia xml network positioning
CONFIG   += c++11
LIBS     += -lX11 -lxcb -lxcb-keysyms -lcups -lsystemd

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theshell
TEMPLATE = app
#QDBUSXML2CPP_ADAPTOR_HEADER_FLAGS = -l NotificationDBus -i notificationdbus.h
#QDBUSXML2CPP_ADAPTOR_SOURCE_FLAGS = -l NotificationDBus -i notificationdbus.h
#DBUS_ADAPTORS = org.freedesktop.Notifications.xml

SOURCES += main.cpp\
        mainwindow.cpp \
    window.cpp \
    menu.cpp \
    endsessionwait.cpp \
    app.cpp \
    background.cpp \
    notificationdbus.cpp \
    notificationdialog.cpp \
    UGlobalHotkey-master/uexception.cpp \
    UGlobalHotkey-master/uglobalhotkeys.cpp \
    UGlobalHotkey-master/ukeysequence.cpp \
    upowerdbus.cpp \
    lockscreen.cpp \
    infopanedropdown.cpp \
    clickablelabel.cpp \
    thewave.cpp \
    thewaveworker.cpp \
    loginsplash.cpp \
    notifications_adaptor.cpp \
    hoverframe.cpp \
    choosebackground.cpp \
    switch.cpp \
    FlowLayout/flowlayout.cpp \
    touchkeyboard.cpp \
    powermanager.cpp \
    segfaultdialog.cpp \
    globalfilter.cpp

HEADERS  += mainwindow.h \
    window.h \
    menu.h \
    endsessionwait.h \
    app.h \
    background.h \
    notificationdbus.h \
    notifications_adaptor.h \
    notificationdialog.h \
    UGlobalHotkey-master/hotkeymap.h \
    UGlobalHotkey-master/uexception.h \
    UGlobalHotkey-master/uglobal.h \
    UGlobalHotkey-master/uglobalhotkeys.h \
    UGlobalHotkey-master/ukeysequence.h \
    upowerdbus.h \
    lockscreen.h \
    infopanedropdown.h \
    clickablelabel.h \
    thewave.h \
    thewaveworker.h \
    loginsplash.h \
    notifications_adaptor.h \
    hoverframe.h \
    choosebackground.h \
    switch.h \
    FlowLayout/flowlayout.h \
    touchkeyboard.h \
    powermanager.h \
    segfaultdialog.h \
    globalfilter.h

FORMS    += mainwindow.ui \
    menu.ui \
    endsessionwait.ui \
    background.ui \
    notificationdialog.ui \
    lockscreen.ui \
    infopanedropdown.ui \
    thewave.ui \
    loginsplash.ui \
    choosebackground.ui \
    touchkeyboard.ui \
    segfaultdialog.ui

DISTFILES += \
    org.freedesktop.Notifications.xml \
    theshell.desktop \
    init_theshell

RESOURCES += \
    resources.qrc
