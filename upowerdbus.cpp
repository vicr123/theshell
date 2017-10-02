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

#include "upowerdbus.h"
#include "power_adaptor.h"

extern void EndSession(EndSessionWait::shutdownType type);
extern NotificationsDBusAdaptor* ndbus;

UPowerDBus::UPowerDBus(QObject *parent) : QObject(parent)
{
    new PowerAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/org/thesuite/Power", "org.thesuite.Power", this);

    connect(ndbus, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(ActionInvoked(uint,QString)));

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
            if (device.path().contains("battery") || device.path().contains("media_player") || device.path().contains("computer") || device.path().contains("phone")) { //This is a battery or media player or tablet computer
                QDBusConnection::systemBus().connect("org.freedesktop.UPower", device.path(),
                                                     "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                                                                      SLOT(DeviceChanged()));

                QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", device.path(), "org.freedesktop.UPower.Device", QDBusConnection::systemBus(), this);
                allDevices.append(i);
            }

            if (device.path().contains("battery")) {
                batteryPath = device;
            }
        }
        DeviceChanged();
    } else {
        emit updateDisplay(tr("Can't get battery information."));
    }
}

void UPowerDBus::DeviceChanged() {
    QStringList displayOutput;

    bool powerStretchMessageNotPrinted = true;
    hasPCBat = false;
    for (QDBusInterface *i : allDevices) {
        //Get the percentage of battery remaining.
        //We do the calculation ourselves because UPower can be inaccurate sometimes
        double percentage;

        //Check that the battery actually reports energy information
        double energyFull = i->property("EnergyFull").toDouble();
        double energy = i->property("Energy").toDouble();
        double energyEmpty = i->property("EnergyEmpty").toDouble();
        if (energyFull == 0 && energy == 0 && energyEmpty == 0) {
            //The battery does not report energy information, so get the percentage from UPower.
            percentage = i->property("Percentage").toDouble();
        } else {
            //Calculate the percentage ourself, and round it to an integer.
            //Add 0.5 because C++ always rounds down.
            percentage = (int) (((energy - energyEmpty) / (energyFull - energyEmpty) * 100) + 0.5);
        }
        if (i->path().contains("battery")) {
            //PC Battery
            if (i->property("IsPresent").toBool()) {
                hasPCBat = true;
                bool showRed = false;
                qulonglong timeToFull = i->property("TimeToFull").toULongLong();
                qulonglong timeToEmpty = i->property("TimeToEmpty").toULongLong();

                //Depending on the state, do different things.
                QString state;
                switch (i->property("State").toUInt()) {
                case 1: //Charging
                    state = " (" + tr("Charging");

                    if (!isCharging) {
                        QString message;

                        if (isPowerStretchOn) {
                            setPowerStretch(false);
                            message = tr("The power cable has been plugged in and the battery is now being charged. Power Stretch has been turned off.");
                        } else {
                            message = tr("The power cable has been plugged in and the battery is now being charged.");
                        }

                        QVariantMap hints;
                        hints.insert("category", "battery.charging");
                        hints.insert("transient", true);
                        hints.insert("sound-file", "qrc:/sounds/charging.wav");
                        ndbus->Notify("theShell", 0, "", tr("Charging"),
                                                       message, QStringList(), hints, 10000);
                    }

                    isCharging = true;
                    isConnectedToPower = true;

                    if (timeToFull != 0) {
                        timeRemain = QDateTime::fromTime_t(timeToFull).toUTC();
                        state.append(" · " + timeRemain.toString("h:mm"));
                    } else {
                        timeRemain = QDateTime(QDate(0, 0, 0));
                    }

                    if (hourBatteryWarning) {
                        hourBatteryWarning = false;
                        halfHourBatteryWarning = false;
                        tenMinuteBatteryWarning = false;
                        ndbus->CloseNotification(batteryLowNotificationNumber);
                        batteryLowNotificationNumber = 0;
                    }
                    state += ")";
                    break;
                case 2: //Discharging
                    //state = " (" + tr("Discharging");
                    state = " (";
                    if (isConnectedToPower) {
                        QVariantMap hints;
                        hints.insert("category", "battery.discharging");
                        hints.insert("transient", true);
                        ndbus->Notify("theShell", 0, "", tr("Discharging"),
                                                       tr("The power cable has been removed, and your PC is now running on battery power."),
                                                       QStringList(), hints, 10000);
                    }
                    isConnectedToPower = false;
                    isCharging = false;

                    if (timeToEmpty != 0) {
                        timeRemain = QDateTime::fromTime_t(timeToEmpty).toUTC();
                        state.append(/*" · " + */timeRemain.toString("h:mm"));

                        if (timeToEmpty <= 600 && tenMinuteBatteryWarning == false) { //Ten minutes left! Critical!
                            QVariantMap hints;
                            hints.insert("urgency", 2);
                            hints.insert("category", "battery.critical");
                            hints.insert("sound-file", "qrc:/sounds/powerlow.wav");

                            QStringList actions;
                            if (!isPowerStretchOn) {
                                actions.append("power-stretch-on");
                                actions.append(tr("Turn on Power Stretch"));
                            }
                            batteryLowNotificationNumber = ndbus->Notify("theShell", batteryLowNotificationNumber, "", tr("Battery Critically Low"),
                                                           tr("You have about 10 minutes of battery remaining."
                                                           " Either plug in your PC or save your work"
                                                           " and power off the PC and change the battery."), actions, hints, 0);

                            tenMinuteBatteryWarning = true;
                            halfHourBatteryWarning = true;
                            hourBatteryWarning = true;
                        } else if (timeToEmpty <= 1800 && halfHourBatteryWarning == false) { //Half hour left! Low!
                            QVariantMap hints;
                            hints.insert("urgency", 2);
                            hints.insert("category", "battery.low");
                            hints.insert("sound-file", "qrc:/sounds/powerlow.wav");

                            QStringList actions;
                            if (!isPowerStretchOn) {
                                actions.append("power-stretch-on");
                                actions.append(tr("Turn on Power Stretch"));
                            }
                            batteryLowNotificationNumber = ndbus->Notify("theShell", batteryLowNotificationNumber, "", tr("Battery Low"),
                                                           tr("You have about half an hour of battery remaining."
                                                           " You should plug in your PC now."), actions, hints, 10000);


                            halfHourBatteryWarning = true;
                            hourBatteryWarning = true;
                        } else if (timeToEmpty <= 3600 && hourBatteryWarning == false) { //One hour left! Warning!
                            QVariantMap hints;
                            hints.insert("urgency", 2);
                            hints.insert("category", "battery.low");
                            hints.insert("sound-file", "qrc:/sounds/powerlow.wav");

                            QStringList actions;
                            if (!isPowerStretchOn) {
                                actions.append("power-stretch-on");
                                actions.append(tr("Turn on Power Stretch"));
                            }
                            batteryLowNotificationNumber = ndbus->Notify("theShell", batteryLowNotificationNumber, "", tr("Battery Warning"),
                                                           tr("You have about an hour of battery remaining."
                                                            " You may want to plug in your PC now."), actions, hints, 10000);
                            hourBatteryWarning = true;
                        }

                        if (halfHourBatteryWarning || tenMinuteBatteryWarning) {
                            showRed = true;
                        }
                    } else {
                        timeRemain = QDateTime(QDate(0, 0, 0));
                    }
                    state += ")";
                    break;
                case 3: //Empty
                    state = " (" + tr("Empty") + ")";
                    break;
                case 4: //Charged
                case 6: //Pending Discharge
                    state = " (" + tr("Full") + ")";
                    if (isCharging) {
                        QVariantMap hints;
                        hints.insert("category", "battery.charged");
                        hints.insert("transient", true);
                        ndbus->Notify("theShell", 0, "", "Battery Charged",
                                                       "The battery has been charged completely."
                                                       , QStringList(), hints, 10000);
                    }
                    isCharging = false;
                    isConnectedToPower = true;
                    timeRemain = QDateTime(QDate(0, 0, 0));
                    break;
                case 5: //Pending Charge
                    state = " (" + tr("Not Charging") + ")";
                    break;
                }

                if (showRed) {
                    displayOutput.append("<span style=\"background-color: red; color: black;\">" + tr("%1% PC Battery%2").arg(QString::number(percentage), state) + "</span>");
                } else {
                    displayOutput.append(tr("%1% PC Battery%2").arg(QString::number(percentage), state));
                }

                if (isPowerStretchOn && powerStretchMessageNotPrinted) {
                    powerStretchMessageNotPrinted = true;
                    displayOutput.append("<span style=\"background-color: orange; color: black;\">" + tr("Power Stretch on") + "</span>");
                }

                batLevel = percentage;
            } else {
                displayOutput.append(tr("No Battery Inserted"));
            }
        } else if (i->path().contains("media_player") || i->path().contains("computer") || i->path().contains("phone")) {
            //This is an external media player (or tablet)
            //Get the model of this media player
            QString model = i->property("Model").toString();

            if (i->property("Serial").toString().length() == 40 && i->property("Vendor").toString().contains("Apple") && QFile("/usr/bin/idevice_id").exists()) { //This is probably an iOS device
                //Get the name of the iOS device
                QProcess iosName;
                iosName.start("idevice_id " + i->property("Serial").toString());
                iosName.waitForFinished();

                QString name(iosName.readAll());
                name = name.trimmed();

                if (name != "" && !name.startsWith("ERROR:")) {
                    model = name;
                }
            }
            if (i->property("State").toUInt() == 0) {
                if (QFile("/usr/bin/thefile").exists()) {
                    displayOutput.append(tr("Pair %1 using theFile to see battery status.").arg(model));
                } else {
                    displayOutput.append(tr("%1 battery unavailable. Device trusted?").arg(model));
                }
            } else {
                QString batteryText;
                batteryText.append(tr("%1% battery on %2").arg(QString::number(percentage), model));
                switch (i->property("State").toUInt()) {
                case 1:
                    batteryText.append(" (" + tr("Charging") + ")");
                    break;
                case 2:
                    batteryText.append(" (" + tr("Discharging") + ")");
                    break;
                case 3:
                    batteryText.append(" (" + tr("Empty") + ")");
                    break;
                case 4:
                case 6:
                    batteryText.append(" (" + tr("Full") + ")");
                    break;
                case 5:
                    batteryText.append(" (" + tr("Not Charging") + ")");
                    break;
                }
                displayOutput.append(batteryText);
            }
        }
    }

    //If KDE Connect is running, check the battery status of connected devices.
    if (QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.kde.kdeconnect")) {
        //Get connected devices
        QDBusMessage devicesMessage = QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon", "devices");
        devicesMessage.setArguments(QVariantList() << true);
        QDBusReply<QStringList> connectedDevices = QDBusConnection::sessionBus().call(devicesMessage, QDBus::Block, 5000);
        if (connectedDevices.isValid()) {
            for (QString device : connectedDevices.value()) {
                QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device");
                QString name = interface.property("name").toString();
                QDBusInterface batteryInterface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device.battery");
                if (batteryInterface.isValid()) {
                    QDBusReply<int> currentCharge = batteryInterface.call("charge");
                    QDBusReply<bool> charging = batteryInterface.call("isCharging");

                    if (currentCharge.isValid()) {
                        if (currentCharge != -1) {
                            QString batteryText;
                            if (charging) {
                                if (currentCharge == 100) {
                                    batteryText = tr("%1% battery on %2 (Full)").arg(QString::number(currentCharge), name);
                                } else {
                                    batteryText = tr("%1% battery on %2 (Charging)").arg(QString::number(currentCharge), name);
                                }
                            } else {
                                batteryText = tr("%1% battery on %2 (Discharging)").arg(QString::number(currentCharge), name);
                            }
                            displayOutput.append(batteryText);
                        }
                    }
                }
            }
        }
    }

    if (displayOutput.count() == 0) {
        hasBat = false;
        emit updateDisplay("");
    } else {
        hasBat = true;
        emit updateDisplay(displayOutput.join(" · "));
    }
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

QDBusObjectPath UPowerDBus::defaultBattery() {
    return batteryPath;
}

QDateTime UPowerDBus::batteryTimeRemaining() {
    return timeRemain;
}

bool UPowerDBus::charging() {
    return isCharging;
}

bool UPowerDBus::powerStretch() {
    return isPowerStretchOn;
}

void UPowerDBus::setPowerStretch(bool on) {
    if (isPowerStretchOn != on) {
        isPowerStretchOn = on;
        emit powerStretchChanged(on);
        this->devicesChanged();
    }
}

void UPowerDBus::ActionInvoked(uint id, QString action_key) {
    if (id == batteryLowNotificationNumber) {
        if (action_key == "power-stretch-on") {
            setPowerStretch(true);
        }
    }
}

bool UPowerDBus::hasPCBattery() {
    return hasPCBat;
}
