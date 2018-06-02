QT -= gui

LIBS += -lcrypt

CONFIG += c++11 console
CONFIG -= app_bundle

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += polkit-qt5-1
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp


blueprint {
    TARGET = ts-mousepass-changeb

    DEFINES += "BLUEPRINT"
} else {
    TARGET = ts-mousepass-change
}

unix {
    target.path = /usr/lib

    suid.path = /usr/lib

    blueprint {
        suid.extra = chmod u+s $(INSTALL_ROOT)/usr/lib/ts-mousepass-changeb
    } else {
        suid.extra = chmod u+s $(INSTALL_ROOT)/usr/lib/ts-mousepass-change
    }

    INSTALLS += target suid
}
