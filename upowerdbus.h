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
public:
    explicit UPowerDBus(NotificationDBus* notificationDBus, QObject *parent = 0);

    bool hasBattery();
    int currentBattery();
    QDBusObjectPath defaultBattery();
    QDateTime batteryTimeRemaining();
    bool charging();
signals:
    void updateDisplay(QString display);
    void batteryChanged(int batteryPercent);
public slots:
    void DeviceChanged();
    void checkUpower();
    void devicesChanged();

private slots:

private:
    QList<QDBusInterface*> allDevices;

    bool hourBatteryWarning = false;
    bool halfHourBatteryWarning = false;
    bool tenMinuteBatteryWarning = false;

    bool isCharging = false;
    bool isConnectedToPower = false;

    int batteryLowNotificationNumber = 0;

    NotificationDBus* notificationDBus;
    bool hasBat;
    int batLevel;

    QDBusUnixFileDescriptor powerInhibit;

    QTimer* checkTimer;
    QSettings settings;

    QDBusObjectPath batteryPath;

    QDateTime timeRemain;

    bool isLidClosed;
};

#endif // UPOWERDBUS_H
