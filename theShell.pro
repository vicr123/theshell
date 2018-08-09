TEMPLATE = subdirs

DISTFILES += \
    version

SUBDIRS += \
    shell \
    startsession \
    polkitagent \
    mousepass

blueprint {
    message(Configuring theShell to be built as blueprint)
    DEFINES += "BLUEPRINT"
} else: construction {
    message(Configuring theShell to be built as construction)
    DEFINES += "CONSTRUCTION"
} else {
    message(Configuring theShell to be built as stable)
}
