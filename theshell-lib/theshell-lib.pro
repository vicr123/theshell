#-------------------------------------------------
#
# Project created by QtCreator 2019-04-01T00:11:20
#
#-------------------------------------------------

QT       += core gui multimedia thelib

blueprint {
    TARGET = theshell-libb
    DEFINES += "BLUEPRINT"
} else {
    TARGET = theshell-lib
}

TEMPLATE = lib

DEFINES += THESHELLLIB_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    mpris/mprisengine.cpp \
    mpris/mprisplayer.cpp \
        notificationspermissionengine.cpp \
    application.cpp \
    qsettingsformats.cpp \
    soundengine.cpp

HEADERS += \
        mpris/mprisengine.h \
        mpris/mprisplayer.h \
        notificationspermissionengine.h \
        theshell-lib_global.h \ 
    application.h \
    qsettingsformats.h \
    soundengine.h

unix {
    target.path = /usr/lib

    header.files = *.h

    blueprint {
        header.path = /usr/include/ts-libb
    } else {
        header.path = /usr/include/ts-lib
    }

    INSTALLS += target header
}
