#-------------------------------------------------
#
# Project created by QtCreator 2019-03-19T23:42:00
#
#-------------------------------------------------

QT       += core gui widgets thelib

TARGET = tspulseaudio
TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += ../../shell/statuscenter/

LIBS += -L$$OUT_PWD/../../theshell-lib/

INCLUDEPATH += $$PWD/../../theshell-lib
DEPENDPATH += $$PWD/../../theshell-lib

blueprint {
    DEFINES += "BLUEPRINT"

    DEFINES += "SHAREDIR=\\\"/usr/share/theshellb/pulseaudiopane/\\\""
} else {
    DEFINES += "SHAREDIR=\\\"/usr/share/theshell/pulseaudiopane/\\\""
}

LIBS += -lKF5PulseAudioQt
INCLUDEPATH += /usr/include/KF5/KF5PulseAudioQt/PulseAudioQt

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    translations.files = translations/*.qm

    blueprint {
        target.path = /usr/lib/theshellb/panes/
        translations.path = /usr/share/theshellb/pulseaudiopane/translations
    } else {
        target.path = /usr/lib/theshell/panes/
        translations.path = /usr/share/theshell/pulseaudiopane/translations
    }
    INSTALLS += target translations
}

HEADERS += \
    cardwidget.h \
    deviceselection.h \
    plugin.h \
    audiopane.h \
    sinkwidget.h \
    sinkinputwidget.h \
    maps.h \
    sourceoutputwidget.h \
    sourcewidget.h

SOURCES += \
    cardwidget.cpp \
    deviceselection.cpp \
    plugin.cpp \
    audiopane.cpp \
    sinkwidget.cpp \
    sinkinputwidget.cpp \
    sourceoutputwidget.cpp \
    sourcewidget.cpp

DISTFILES += \
    PulseaudioPane.json

FORMS += \
    audiopane.ui \
    cardwidget.ui \
    deviceselection.ui \
    sinkwidget.ui \
    sinkinputwidget.ui \
    sourceoutputwidget.ui \
    sourcewidget.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libpulse libpulse-mainloop-glib
