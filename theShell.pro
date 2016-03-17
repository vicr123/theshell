#-------------------------------------------------
#
# Project created by QtCreator 2016-02-17T17:22:06
#
#-------------------------------------------------

QT       += core gui x11extras dbus
CONFIG   += C++11
LIBS     += -lxcb -lX11


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = theshell
TEMPLATE = app
DBUS_ADAPTORS = org.freedesktop.Notifications.xml

SOURCES += main.cpp\
        mainwindow.cpp \
    window.cpp \
    menu.cpp \
    endsessionwait.cpp \
    app.cpp \
    background.cpp \
    notificationdbus.cpp \
    notificationdialog.cpp \
    #UGlobalHotkey-master/uexception.cpp \
    #UGlobalHotkey-master/uglobalhotkeys.cpp \
    #UGlobalHotkey-master/ukeysequence.cpp \
    upowerdbus.cpp \
    lockscreen.cpp \
    infopanedropdown.cpp \
    clickablelabel.cpp \
    nativeeventfilter.cpp

HEADERS  += mainwindow.h \
    window.h \
    menu.h \
    endsessionwait.h \
    app.h \
    background.h \
    notificationdbus.h \
    notifications_adaptor.h \
    notificationdialog.h \
    #UGlobalHotkey-master/hotkeymap.h \
    #UGlobalHotkey-master/uexception.h \
    #UGlobalHotkey-master/uglobal.h \
    #UGlobalHotkey-master/uglobalhotkeys.h \
    #UGlobalHotkey-master/ukeysequence.h \
    upowerdbus.h \
    lockscreen.h \
    infopanedropdown.h \
    clickablelabel.h \
    nativeeventfilter.h

FORMS    += mainwindow.ui \
    menu.ui \
    endsessionwait.ui \
    background.ui \
    notificationdialog.ui \
    lockscreen.ui \
    infopanedropdown.ui

DISTFILES += \
    org.freedesktop.Notifications.xml
