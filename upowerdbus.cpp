#include "upowerdbus.h"

UPowerDBus::UPowerDBus(NotificationDBus* notificationDBus, QObject *parent) : QObject(parent)
{
    this->notificationDBus = notificationDBus;
    QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> reply = i->call("EnumerateDevices");

    if (reply.isValid()) {
        for (QDBusObjectPath device : reply.value()) {
            if (device.path().contains("battery")) {
                QDBusConnection::systemBus().connect("org.freedesktop.UPower", device.path(),
                                                     "org.freedesktop.UPower.Device", "Changed", this,
                                                                      SLOT(DeviceChanged()));

                QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", device.path(), "org.freedesktop.UPower.Device", QDBusConnection::systemBus(), this);
                allDevices.append(i);
            }
        }
        if (allDevices.length() == 0) {
            hasBat = false;
        } else {
            hasBat = true;
            QTimer *t = new QTimer(this);
            t->setInterval(60000);
            connect(t, SIGNAL(timeout()), this, SLOT(DeviceChanged()));
            t->start();
        }
        //DeviceChanged();
    } else {
        emit updateDisplay("Can't get battery information.");
    }
}

void UPowerDBus::DeviceChanged() {
    for (QDBusInterface *i : allDevices) {
        double percentage = i->property("Percentage").toDouble();
        qulonglong timeToFull = i->property("TimeToFull").toULongLong();
        qulonglong timeToEmpty = i->property("TimeToEmpty").toULongLong();
        QString state;
        switch (i->property("State").toUInt()) {
        case 1:

            state = " and charging";
            if (timeToFull != 0) {
                state.append(" (" + QDateTime::fromTime_t(timeToFull).toUTC().toString("h:mm") + ")");
            }

            if (hourBatteryWarning) {
                hourBatteryWarning = false;
                halfHourBatteryWarning = false;
                tenMinuteBatteryWarning = false;
                this->notificationDBus->CloseNotification(batteryLowNotificationNumber);
                batteryLowNotificationNumber = 0;
            }
            break;
        case 2:
            state = " battery left";
                if (timeToEmpty != 0) {
                    state.append(" (" + QDateTime::fromTime_t(timeToEmpty).toUTC().toString("h:mm") + ")");

                    if (timeToEmpty <= 600 && tenMinuteBatteryWarning == false) { //Ten minutes left! Critical!
                        /*QSystemTrayIcon *trayicon = new QSystemTrayIcon(this);

                        trayicon->showMessage("Battery Critically Low",
                                                          "You have about 10 minutes of battery remaining."
                                                          " Either plug in your PC or save your work"
                                                          " and power off the PC and change the battery.",
                                                          QSystemTrayIcon::Critical, 0);
                        trayicon->hide();*/
                        QVariantMap hints;
                        hints.insert("urgency", 2);
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Critically Low",
                                                       "You have about 10 minutes of battery remaining."
                                                       " Either plug in your PC or save your work"
                                                       " and power off the PC and change the battery.", QStringList(), hints, 0);

                        tenMinuteBatteryWarning = true;
                        halfHourBatteryWarning = true;
                        hourBatteryWarning = true;
                    } else if (timeToEmpty <= 1800 && halfHourBatteryWarning == false) { //Half hour left! Low!
                        /*QSystemTrayIcon *trayicon = new QSystemTrayIcon(this);
                        trayicon->show();
                        trayicon->showMessage("Battery Low",
                                                          "You have about half an hour of battery remaining."
                                                          " You should plug in your PC now.",
                                                          QSystemTrayIcon::Warning, 10000);
                        trayicon->hide();*/
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Low",
                                                       "You have about half an hour of battery remaining."
                                                       " You should plug in your PC now.", QStringList(), QVariantMap(), 10000);


                        halfHourBatteryWarning = true;
                        hourBatteryWarning = true;
                    } else if (timeToEmpty <= 3600 && hourBatteryWarning == false) { //One hour left! Warning!
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Warning",
                                                       "You have about an hour of battery remaining."
                                                        " You may want to plug in your PC now.", QStringList(), QVariantMap(), 10000);


                        /*QSystemTrayIcon *trayicon = new QSystemTrayIcon(this);
                        trayicon->showMessage("Battery Warning",
                                                          "You have about an hour of battery remaining."
                                                          " You may want to plug in your PC now.",
                                                          QSystemTrayIcon::Information, 10000);
                        trayicon->hide();*/
                        hourBatteryWarning = true;
                    }
                }
            break;
        case 3:
            state = " - Empty";
            break;
        case 4:
            state = " Fully Charged.";
            break;
        case 5:
            state = " Not charging";
            break;
        case 6:
            state = " Fully Charged.";
            break;
        }

        emit updateDisplay(QString::number(percentage) + "%" + state);
    }
}

bool UPowerDBus::hasBattery() {
    return hasBat;
}
