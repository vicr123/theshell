TEMPLATE = subdirs

if (exists("/usr/lib/libcups.so") | enable-print-module) {
    SUBDIRS += \
        PrinterPane
} else {
    message("CUPS not detected; printing module will not be built!")
    message("To force building of printing module, pass CONFIG+=enable-print-module to qmake")
}

SUBDIRS += \
    BluetoothPane \
    OverviewPane \
    NotificationsPane \
    KDEConnectPane \
    NetworkPane \
    PulseaudioPane
