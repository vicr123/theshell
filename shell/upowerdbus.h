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

#ifndef UPOWERDBUS_H
#define UPOWERDBUS_H

#include <QObject>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QSoundEffect>
#include "endsessionwait.h"
#include "notificationsWidget/notificationsdbusadaptor.h"

#include <X11/Xlib.h>

#define Bool int
#define Status int
#include <X11/extensions/scrnsaver.h>
#include <X11/extensions/dpms.h>
#undef Bool
#undef Status

class UPowerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.Power")
public:
    explicit UPowerDBus(QObject *parent = 0);

    bool hasBattery();
    bool hasPCBattery();
    int currentBattery();
    QDBusObjectPath defaultBattery();
    QDateTime batteryTimeRemaining();
    bool charging();

    Q_SCRIPTABLE bool powerStretch();

signals:
    void updateDisplay(QString display);
    void batteryChanged(int batteryPercent);

    Q_SCRIPTABLE void powerStretchChanged(bool powerStretch);

public slots:
    void DeviceChanged();
    void checkUpower();
    void devicesChanged();
    void setPowerStretch(bool on);
    void queryIdleState();

private slots:
    void ActionInvoked(uint id, QString action_key);

private:
    QList<QDBusInterface*> allDevices;

    bool hourBatteryWarning = false;
    bool halfHourBatteryWarning = false;
    bool tenMinuteBatteryWarning = false;

    bool isCharging = false;
    bool isConnectedToPower = false;

    bool isPowerStretchOn = false;

    uint batteryLowNotificationNumber = 0;

    bool hasBat;
    bool hasPCBat;
    int batLevel;

    QDBusUnixFileDescriptor powerInhibit;

    QTimer* checkTimer;
    QSettings settings;

    QDBusObjectPath batteryPath;

    QDateTime timeRemain;

    bool isLidClosed = false;

    bool idleScreen = false, idleSuspend = false;
};

#endif // UPOWERDBUS_H
