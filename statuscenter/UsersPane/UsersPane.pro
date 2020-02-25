#-------------------------------------------------
#
# Project created by QtCreator 2019-03-26T23:10:56
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib

TARGET = tsusers
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

unix {
    PKGCONFIG += polkit-qt5-1
    CONFIG += link_pkgconfig
}

blueprint {
    DEFINES += "BLUEPRINT"

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/users/\\\""
} else {
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/users/\\\""
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
        translations.path = /usr/share/theshellb/users/translations
    } else {
        target.path = /usr/lib/theshell/panes/
        translations.path = /usr/share/theshell/users/translations
    }
    INSTALLS += target translations
}

HEADERS += \
    adduserdialog.h \
    changepassworddialog.h \
    changerealnamedialog.h \
    deleteuserdialog.h \
    lockuserdialog.h \
    plugin.h \
    user.h \
    usersmodel.h \
    userspane.h \
    usertypedialog.h

SOURCES += \
    adduserdialog.cpp \
    changepassworddialog.cpp \
    changerealnamedialog.cpp \
    deleteuserdialog.cpp \
    lockuserdialog.cpp \
    plugin.cpp \
    user.cpp \
    usersmodel.cpp \
    userspane.cpp \
    usertypedialog.cpp

DISTFILES += \
    UsersPane.json

FORMS += \
    adduserdialog.ui \
    changepassworddialog.ui \
    changerealnamedialog.ui \
    deleteuserdialog.ui \
    lockuserdialog.ui \
    userspane.ui \
    usertypedialog.ui
