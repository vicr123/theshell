#-------------------------------------------------
#
# Project created by QtCreator 2018-07-05T10:27:46
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib positioning network multimedia

TARGET = tsoverview
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DISTFILES += OverviewPane.json \
    OverviewPane.json

unix {
    blueprint {
        target.path = /usr/lib/theshellb/panes/
    } else {
        target.path = /usr/lib/theshell/panes/
    }
    INSTALLS += target
}

HEADERS += \
    plugin.h \
    overview.h \
    overviewsettings.h \
    Timers/timerpage.h \
    Timers/timeritem.h \
    Stopwatch/stopwatchpage.h \
    Reminders/reminderspage.h \
    Reminders/reminderslistmodel.h

SOURCES += \
    plugin.cpp \
    overview.cpp \
    overviewsettings.cpp \
    Timers/timerpage.cpp \
    Timers/timeritem.cpp \
    Stopwatch/stopwatchpage.cpp \
    Reminders/reminderspage.cpp \
    Reminders/reminderslistmodel.cpp

FORMS += \
    overview.ui \
    overviewsettings.ui \
    Timers/timerpage.ui \
    Timers/timeritem.ui \
    Stopwatch/stopwatchpage.ui \
    Reminders/reminderspage.ui

