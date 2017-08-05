/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef DBUSEVENTS_H
#define DBUSEVENTS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QApplication>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDebug>
#include <QDBusInterface>
#include <QSettings>
#include <QTimer>
#include <QSoundEffect>
#include "notificationdbus.h"
#include "newmedia.h"

class DbusEvents : public QObject
{
    Q_OBJECT
public:
    explicit DbusEvents(QObject *parent = 0);

signals:

public slots:
    void LockScreen();

    void UnlockScreen();

    void NewUdisksInterface(QDBusObjectPath path);

    void RemoveUdisksInterface(QDBusObjectPath path, QStringList interfaces);

    void NotificationAction(uint id, QString key);

    void DetectNewDevices();

private slots:
    void SleepingNow();

private:
    QProcess* LockScreenProcess = NULL;

    QSettings settings;
    QStringList connectediOSDevices;
    QMap<uint, QDBusObjectPath> notificationIds;
};

#endif // DBUSEVENTS_H
