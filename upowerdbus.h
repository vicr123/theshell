#ifndef UPOWERDBUS_H
#define UPOWERDBUS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QSoundEffect>
#include "notificationdbus.h"

class UPowerDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.Power")
public:
    explicit UPowerDBus(NotificationDBus* notificationDBus, QObject *parent = 0);

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

    NotificationDBus* notificationDBus;
    bool hasBat;
    bool hasPCBat;
    int batLevel;

    QDBusUnixFileDescriptor powerInhibit;

    QTimer* checkTimer;
    QSettings settings;

    QDBusObjectPath batteryPath;

    QDateTime timeRemain;

    bool isLidClosed;
};

#endif // UPOWERDBUS_H
