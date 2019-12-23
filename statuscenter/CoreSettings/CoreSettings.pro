#-------------------------------------------------
#
# Project created by QtCreator 2019-03-23T12:37:45
#
#-------------------------------------------------

QT       += core gui widgets dbus thelib svg x11extras

TARGET = tscoresettings
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += x11 xi xorg-libinput polkit-qt5-1
}

blueprint {
    DEFINES += "BLUEPRINT"

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/coresettings/\\\""
} else {
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/coresettings/\\\""
}
SHARE_APP_NAME = theshell/coresettings

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    blueprint {
        target.path = /usr/lib/theshellb/panes/
    } else {
        target.path = /usr/lib/theshell/panes/
    }
    INSTALLS += target
}

HEADERS += \
    DateTime/datetimepane.h \
    DateTime/timezonesmodel.h \
    Input/shortcutedit.h \
    Input/shortcutpane.h \
    Locale/addlocaledialog.h \
    Locale/localepane.h \
    plugin.h \
    Input/inputpane.h \
    Input/keyboardpane.h \
    Input/gesturepane.h \
    Input/mousepane.h \
    Input/mousepanetester.h \
    theme/colourspane.h \
    theme/iconpane.h \
    theme/themepane.h \
    theme/widgetstylepane.h \
    theme/widgetstylepreview.h

SOURCES += \
    DateTime/datetimepane.cpp \
    DateTime/timezonesmodel.cpp \
    Input/shortcutedit.cpp \
    Input/shortcutpane.cpp \
    Locale/addlocaledialog.cpp \
    Locale/localepane.cpp \
    plugin.cpp \
    Input/inputpane.cpp \
    Input/keyboardpane.cpp \
    Input/gesturepane.cpp \
    Input/mousepane.cpp \
    Input/mousepanetester.cpp \
    theme/colourspane.cpp \
    theme/iconpane.cpp \
    theme/themepane.cpp \
    theme/widgetstylepane.cpp \
    theme/widgetstylepreview.cpp

DISTFILES += \
    CoreSettings.json

FORMS += \
    DateTime/datetimepane.ui \
    Input/inputpane.ui \
    Input/keyboardpane.ui \
    Input/gesturepane.ui \
    Input/mousepane.ui \
    Input/shortcutpane.ui \
    Locale/addlocaledialog.ui \
    Locale/localepane.ui \
    theme/colourspane.ui \
    theme/iconpane.ui \
    theme/themepane.ui \
    theme/widgetstylepane.ui \
    theme/widgetstylepreview.ui

RESOURCES += \
    coresettings_resources.qrc

