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
#include "notificationdbus.h"

class UPowerDBus : public QObject
{
    Q_OBJECT
public:
    explicit UPowerDBus(NotificationDBus* notificationDBus, QObject *parent = 0);

signals:
    void updateDisplay(QString display);
public slots:
    void DeviceChanged();

private slots:

private:
    QList<QDBusInterface*> allDevices;

    bool hourBatteryWarning = false;
    bool halfHourBatteryWarning = false;
    bool tenMinuteBatteryWarning = false;

    int batteryLowNotificationNumber = 0;

    NotificationDBus* notificationDBus;
};

#endif // UPOWERDBUS_H
