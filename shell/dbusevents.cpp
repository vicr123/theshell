/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "dbusevents.h"

#include "notificationsdbusadaptor.h"

DbusEvents::DbusEvents(QObject *parent) : QObject(parent)
{
    qint64 pid = QApplication::applicationPid(); //Get this PID
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
                                         "InterfacesAdded", this, SLOT(NewUdisksInterface(QDBusObjectPath)));
    //QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager",
    //                                     "InterfacesRemoved", this, SLOT(RemoveUdisksInterface(QDBusObjectPath,QStringList)));

    connect(NotificationsDBusAdaptor::instance(), SIGNAL(ActionInvoked(uint,QString)), this, SLOT(NotificationAction(uint,QString)));

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
    if (LockScreenProcess == nullptr) {
        LockScreenProcess = new QProcess();
        connect(LockScreenProcess, (void(QProcess::*)(int, QProcess::ExitStatus)) &QProcess::finished, [=] {
            LockScreenProcess->deleteLater();
            LockScreenProcess = nullptr; //Delete Process
        });
        LockScreenProcess->start("/usr/lib/tsscreenlock"); //Lock Screen
    }
}

void DbusEvents::UnlockScreen() {
    if (LockScreenProcess != nullptr) {
        LockScreenProcess->terminate(); //Kill Process
        //Process will be deleted in the finished() signal
    }
}

void DbusEvents::SleepingNow() {
    if (settings.value("lockScreen/showOnSuspend", true).toBool()) {
        LockScreen();
    }
}

void DbusEvents::NewUdisksInterface(QDBusObjectPath path) {
    if (path.path().startsWith("/org/freedesktop/UDisks2/drives")) {
        if (settings.value("notifications/mediaInsert", true).toBool()) {
            QDBusInterface interface("org.freedesktop.UDisks2", path.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

            QString deviceName = interface.property("Model").toString();
            QStringList actions;
            actions.append("action");
            actions.append(tr("Perform Action..."));
            QVariantMap hints;
            hints.insert("transient", true);
            hints.insert("category", "device.added");
            hints.insert("sound-file", "qrc:/sounds/media-insert.wav");
            NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("%1 Connected").arg(deviceName), tr("%1 has been connected to this PC.").arg(deviceName), actions, hints, -1)->then([=](uint id) {
                notificationIds.insert(id, path);
            });
        }
    }
}

void DbusEvents::RemoveUdisksInterface(QDBusObjectPath path, QStringList interfaces) {
    Q_UNUSED(path)

    if (interfaces.contains("org.freedesktop.UDisks2.Drive")) {
        QSoundEffect* mediaSound = new QSoundEffect();
        mediaSound->setSource(QUrl("qrc:/sounds/media-remove.wav"));
        mediaSound->play();
        connect(mediaSound, SIGNAL(playingChanged()), mediaSound, SLOT(deleteLater()));
    }
}

void DbusEvents::NotificationAction(uint id, QString key) {
    if (notificationIds.keys().contains(id)) {
        if (key == "action") {
            QDBusInterface interface("org.freedesktop.UDisks2", notificationIds.value(id).path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

            QString description = tr("%1 was just connected. What do you want to do?").arg(interface.property("Model").toString());
            NewMedia* mediaWindow = new NewMedia(description);
            mediaWindow->show();
        }
    }
}

void DbusEvents::DetectNewDevices() {
    if (QFile("/usr/bin/idevice_id").exists()) {
        if (settings.value("notifications/mediaInsert", true).toBool()) {
            QProcess deviceId;
            deviceId.start("/usr/bin/idevice_id -l");
            deviceId.waitForFinished();

            //Detect disconnected devices
            QStringList foundDevices = QString(deviceId.readAll()).split("\n", QString::SkipEmptyParts);
            for (QString device : connectediOSDevices) {
                if (!foundDevices.contains(device)) {
                    connectediOSDevices.removeOne(device);

                    QSoundEffect* mediaSound = new QSoundEffect();
                    mediaSound->setSource(QUrl("qrc:/sounds/media-remove.wav"));
                    mediaSound->play();
                    connect(mediaSound, SIGNAL(playingChanged()), mediaSound, SLOT(deleteLater()));
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
                        deviceName = tr("iOS Device");
                    }

                    QVariantMap hints;
                    hints.insert("transient", true);
                    hints.insert("category", "device.added");
                    hints.insert("sound-file", "qrc:/sounds/media-insert.wav");
                    NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("%1 Connected").arg(deviceName), tr("%1 has been connected to this PC.").arg(deviceName), QStringList(), hints, -1);
                    connectediOSDevices.append(device);
                }
            }
        }
    }
}
