#include "upowerdbus.h"

extern void EndSession(EndSessionWait::shutdownType type);
UPowerDBus::UPowerDBus(NotificationDBus* notificationDBus, QObject *parent) : QObject(parent)
{
    this->notificationDBus = notificationDBus;

    //Inhibit logind's handling of some power events
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Inhibit");
    message.setArguments(QList<QVariant>() << "handle-power-key:handle-suspend-key:handle-lid-switch" << "theShell" << "theShell Handles Hardware Power Keys" << "block");
    QDBusReply<QDBusUnixFileDescriptor> inhibitReply = QDBusConnection::systemBus().call(message);
    powerInhibit = inhibitReply.value();

    //Set up timer to check UPower properties
    checkTimer = new QTimer(this);
    checkTimer->setInterval(1000);
    connect(checkTimer, SIGNAL(timeout()), this, SLOT(checkUpower()));
    connect(checkTimer, SIGNAL(timeout()), this, SLOT(DeviceChanged()));
    checkTimer->start();

    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", "DeviceAdded", this, SLOT(devicesChanged()));
    QDBusConnection::systemBus().connect("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", "DeviceRemoved", this, SLOT(devicesChanged()));

    devicesChanged();
}

void UPowerDBus::devicesChanged() {
    allDevices.clear();
    QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> reply = i->call("EnumerateDevices"); //Get all devices

    if (reply.isValid()) { //Check if the reply is ok
        for (QDBusObjectPath device : reply.value()) {
            if (device.path().contains("battery") || device.path().contains("media_player") || device.path().contains("computer")) { //This is a battery or media player or tablet computer
                QDBusConnection::systemBus().connect("org.freedesktop.UPower", device.path(),
                                                     "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                                                      SLOT(DeviceChanged()));

                QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", device.path(), "org.freedesktop.UPower.Device", QDBusConnection::systemBus(), this);
                allDevices.append(i);
            }
        }
        if (allDevices.length() == 0) {
            hasBat = false;
        } else {
            hasBat = true;
        }
    } else {
        emit updateDisplay("Can't get battery information.");
    }
}

void UPowerDBus::DeviceChanged() {
    QString displayOutput;
    for (QDBusInterface *i : allDevices) {
        double percentage = i->property("Percentage").toDouble();
        if (i->path().contains("battery")) {
            //PC Battery
            qulonglong timeToFull = i->property("TimeToFull").toULongLong();
            qulonglong timeToEmpty = i->property("TimeToEmpty").toULongLong();

            //Depending on the state, do different things.
            QString state;
            switch (i->property("State").toUInt()) {
            case 1: //Charging
                state = " (Charging";

                if (!isCharging) {
                    QVariantMap hints;
                    hints.insert("category", "battery.charging");
                    hints.insert("transient", true);
                    this->notificationDBus->Notify("theShell", 0, "", "Charging",
                                                   "The power cable has been plugged in and the battery is now being charged.",
                                                   QStringList(), hints, 10000);

                    QSoundEffect* chargingSound = new QSoundEffect();
                    chargingSound->setSource(QUrl("qrc:/sounds/charging.wav"));
                    chargingSound->play();
                    connect(chargingSound, SIGNAL(playingChanged()), chargingSound, SLOT(deleteLater()));
                }

                isCharging = true;
                isConnectedToPower = true;

                if (timeToFull != 0) {
                    state.append(" - " + QDateTime::fromTime_t(timeToFull).toUTC().toString("h:mm"));
                }

                if (hourBatteryWarning) {
                    hourBatteryWarning = false;
                    halfHourBatteryWarning = false;
                    tenMinuteBatteryWarning = false;
                    this->notificationDBus->CloseNotification(batteryLowNotificationNumber);
                    batteryLowNotificationNumber = 0;
                }
                state += ")";
                break;
            case 2: //Discharging
                state = " (Discharging";
                if (isConnectedToPower) {
                    QVariantMap hints;
                    hints.insert("category", "battery.discharging");
                    hints.insert("transient", true);
                    this->notificationDBus->Notify("theShell", 0, "", "Discharging",
                                                   "The power cable has been removed, and your PC is now running on battery power.",
                                                   QStringList(), hints, 10000);
                }
                isConnectedToPower = false;
                isCharging = false;

                if (timeToEmpty != 0) {
                    state.append(" - " + QDateTime::fromTime_t(timeToEmpty).toUTC().toString("h:mm"));

                    if (timeToEmpty <= 600 && tenMinuteBatteryWarning == false) { //Ten minutes left! Critical!
                        QVariantMap hints;
                        hints.insert("urgency", 2);
                        hints.insert("category", "battery.critical");
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Critically Low",
                                                       "You have about 10 minutes of battery remaining."
                                                       " Either plug in your PC or save your work"
                                                       " and power off the PC and change the battery.", QStringList(), hints, 0);

                        tenMinuteBatteryWarning = true;
                        halfHourBatteryWarning = true;
                        hourBatteryWarning = true;
                    } else if (timeToEmpty <= 1800 && halfHourBatteryWarning == false) { //Half hour left! Low!
                        QVariantMap hints;
                        hints.insert("category", "battery.low");
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Low",
                                                       "You have about half an hour of battery remaining."
                                                       " You should plug in your PC now.", QStringList(), hints, 10000);


                        halfHourBatteryWarning = true;
                        hourBatteryWarning = true;
                    } else if (timeToEmpty <= 3600 && hourBatteryWarning == false) { //One hour left! Warning!
                        QVariantMap hints;
                        hints.insert("urgency", 2);
                        hints.insert("category", "battery.low");
                        batteryLowNotificationNumber = this->notificationDBus->Notify("theShell", batteryLowNotificationNumber, "", "Battery Warning",
                                                       "You have about an hour of battery remaining."
                                                        " You may want to plug in your PC now.", QStringList(), hints, 10000);
                        hourBatteryWarning = true;
                    }
                }
                state += ")";
                break;
            case 3: //Empty
                state = " (Empty)";
                break;
            case 4: //Charged
                state = " (Full)";
                if (isCharging) {
                    QVariantMap hints;
                    hints.insert("category", "battery.charged");
                    hints.insert("transient", true);
                    this->notificationDBus->Notify("theShell", 0, "", "Battery Charged",
                                                   "The battery has been charged completely."
                                                   , QStringList(), hints, 10000);
                }
                isCharging = false;
                isConnectedToPower = true;
                break;
            case 5: //Pending Charge
                state = " (Not Charging)";
                break;
            case 6: //Pending Discharge
                state = " (Full)";
                if (isCharging) {
                    QVariantMap hints;
                    hints.insert("category", "battery.charged");
                    hints.insert("transient", true);
                    this->notificationDBus->Notify("theShell", 0, "", "Battery Charged",
                                                   "The battery has been charged completely."
                                                   , QStringList(), hints, 10000);
                }
                isCharging = false;
                isConnectedToPower = true;
                break;
            }

            displayOutput.append(QString::number(percentage) + "% PC Battery" + state);
            batLevel = percentage;
        } else if (i->path().contains("media_player") || i->path().contains("computer")) {
            //This is an external media player (or tablet)
            if (i->property("State").toUInt() == 0) {
                displayOutput.append(" · " + i->property("Model").toString() + " battery unavailable. Device paired?");
            } else {
                displayOutput.append(" · " + QString::number(percentage) + "% battery on " + i->property("Model").toString());
                switch (i->property("State").toUInt()) {
                case 1:
                    displayOutput.append(" (Charging)");
                    break;
                case 2:
                    displayOutput.append(" (Discharging)");
                    break;
                case 3:
                    displayOutput.append(" (Empty)");
                    break;
                case 4:
                case 6:
                    displayOutput.append(" (Full)");
                    break;
                case 5:
                    displayOutput.append(" (Not Charging)");
                    break;
                }
            }
        }
    }
    emit updateDisplay(displayOutput);
}

bool UPowerDBus::hasBattery() {
    return hasBat;
}

int UPowerDBus::currentBattery() {
    return batLevel;
}

void UPowerDBus::checkUpower() {
    QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus(), this);
    if (i->property("LidIsClosed").toBool()) { //Is the lid closed?
        if (!this->isLidClosed) { //Has the value changed?
            this->isLidClosed = true;
            if (QApplication::screens().count() == 1) { //How many monitors do we have?
                //If we only have one montior, suspend the PC.
                EndSession(EndSessionWait::suspend);
            }
        }
    } else {
        if (this->isLidClosed) {
            this->isLidClosed = false;
        }
    }
}
