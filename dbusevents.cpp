#include "dbusevents.h"

DbusEvents::DbusEvents(NotificationDBus* notifications, QObject *parent) : QObject(parent)
{
    qint64 pid = QApplication::applicationPid(); //Get this PID
    this->notificationEngine = notifications;
    QDBusMessage sessionRequest = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                                 "/org/freedesktop/login1",
                                                                 "org.freedesktop.login1.Manager", "GetSessionByPid"); //Get this session
    sessionRequest.arguments().append(pid);
    QDBusReply<QDBusObjectPath> currentSessionPath = QDBusConnection::systemBus().call(sessionRequest); //Get this session
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Lock",
                                        this, SLOT(LockScreen())); //Register Lock
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Unlock",
                                        this, SLOT(UnlockScreen())); //Register Unlock

    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        "/org/freedesktop/login1",
                                        "org.freedesktop.login1.Manager", "PrepareForSleep",
                                        this, SLOT(SleepingNow())); //Register Sleep

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager",
                                         "InterfacesAdded", "oa{sa{sv}}", this, SLOT(NewUdisksInterface(QDBusObjectPath)));

    if (QFile("/usr/bin/idevice_id").exists()) {
        QProcess deviceId;
        deviceId.start("/usr/bin/idevice_id -l");
        deviceId.waitForFinished();

        connectediOSDevices = QString(deviceId.readAll()).split("\n", QString::SkipEmptyParts);
    }

    QTimer* driveDetect = new QTimer;
    driveDetect->setInterval(1000);
    connect(driveDetect, SIGNAL(timeout()), this, SLOT(DetectNewDevices()));
    driveDetect->start();
}

void DbusEvents::LockScreen() {
    if (LockScreenProcess == NULL) {
        LockScreenProcess = new QProcess();
        connect(LockScreenProcess, (void(QProcess::*)(int, QProcess::ExitStatus)) &QProcess::finished, [=]() {
            delete LockScreenProcess;
            LockScreenProcess = NULL; //Delete Process
        });
        LockScreenProcess->start("/usr/lib/tsscreenlock"); //Lock Screen
    }
}

void DbusEvents::UnlockScreen() {
    if (LockScreenProcess != NULL) {
        LockScreenProcess->terminate(); //Kill Process
        //Process will be deleted in the finished() signal
    }
}

void DbusEvents::SleepingNow() {
    LockScreen();
}

void DbusEvents::NewUdisksInterface(QDBusObjectPath path) {
    if (path.path().startsWith("/org/freedesktop/UDisks2/drives")) {
        if (settings.value("notifications/mediaInsert", true).toBool()) {
            QDBusInterface interface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

            QString description = interface.property("Model").toString() + " was just connected. What do you want to do?";
            NewMedia* mediaWindow = new NewMedia(description);
            mediaWindow->show();
        }
    }
}

void DbusEvents::DetectNewDevices() {
    if (QFile("/usr/bin/idevice_id").exists()) {
        QProcess deviceId;
        deviceId.start("/usr/bin/idevice_id -l");
        deviceId.waitForFinished();

        //Detect disconnected devices
        QStringList foundDevices = QString(deviceId.readAll()).split("\n", QString::SkipEmptyParts);
        for (QString device : connectediOSDevices) {
            if (!foundDevices.contains(device)) {
                connectediOSDevices.removeOne(device);
            }
        }

        //Detect connected device
        for (QString device : foundDevices) {
            if (!connectediOSDevices.contains(device)) {
                QProcess deviceNameProc;
                deviceNameProc.start("/usr/bin/idevice_id " + device);
                deviceNameProc.waitForFinished();

                QString deviceName = deviceNameProc.readAll().trimmed();
                if (deviceName == "") {
                    deviceName = "iOS Device";
                }

                QVariantMap hints;
                hints.insert("transient", true);
                hints.insert("category", "device.added");
                notificationEngine->Notify("theShell", 0, "", deviceName + " Connected", deviceName + " has been connected to this PC.", QStringList(), hints, -1);
                connectediOSDevices.append(device);
            }
        }
    }
}
