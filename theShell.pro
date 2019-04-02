TEMPLATE = subdirs

DISTFILES += \
    version

shellproj.subdir = shell
shellproj.depends = theshell-lib

statcenterproj.subdir = statuscenter
statcenterproj.depends = theshell-lib

daemonproj.subdir = daemons
daemonproj.depends = theshell-lib

SUBDIRS += \
    shellproj \
    startsession \
    statcenterproj \
    polkitagent \
    mousepass \
    daemonproj \
    theshell-lib

blueprint {
    message(Configuring theShell to be built as blueprint)
    DEFINES += "BLUEPRINT"
} else {
    message(Configuring theShell to be built as stable)
}
