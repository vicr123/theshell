TEMPLATE = subdirs

DISTFILES += \
    version

SUBDIRS += \
    shell \
    startsession \
    polkitagent \
    mousepass \
    statuscenter \
    daemons \
    theshell-lib

statuscenter.depends = theshell-lib
shell.depends = theshell-lib

blueprint {
    message(Configuring theShell to be built as blueprint)
    DEFINES += "BLUEPRINT"
} else {
    message(Configuring theShell to be built as stable)
}
