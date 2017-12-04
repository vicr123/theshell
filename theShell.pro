TEMPLATE = subdirs

DISTFILES += \
    version

SUBDIRS += \
    shell \
    startsession

blueprint {
    message(Configuring theShell to be built as blueprint)
    DEFINES += "BLUEPRINT"
} else {
    message(Configuring theShell to be built as stable)
}
