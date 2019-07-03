/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "devicepanel.h"
#include "ui_devicepanel.h"

#include <the-libs_global.h>
#include <QDBusInterface>
#include <QBoxLayout>
#include <QPushButton>
#include <QMetaType>
#include <QDBusMetaType>
#include <tpopover.h>
#include <tnotification.h>
#include "enums.h"
#include "popovers/simpinrequest.h"
#include "connectioneditor/devicesettings.h"

#include <modem.h>
#include <modem3gpp.h>
#include <sim.h>

struct DevicePanelPrivate {
    QDBusInterface* deviceInterface;
    QDBusInterface* nmInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    QDBusObjectPath device;
    QWidget* popoverOn;

    QIcon chunkIcon;
    QString chunkText;

    DevicePanel::DevicePanelType type = DevicePanel::Unknown;

    bool modemConnected = false;
    bool promptedForPin = false;
};

DevicePanel::DevicePanel(QDBusObjectPath device, QWidget* popoverOnWidget, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::DevicePanel) {

    ui->setupUi(this);
    d = new DevicePanelPrivate();

    d->deviceInterface = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
    d->device = device;
    d->popoverOn = popoverOnWidget;
    QDBusConnection::systemBus().connect(d->deviceInterface->service(), device.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateInfo()));

    updateInfo();
}

DevicePanel::~DevicePanel() {
    d->deviceInterface->deleteLater();
    d->nmInterface->deleteLater();
    delete d;
}

DevicePanel::DevicePanelType DevicePanel::deviceType() {
    return d->type;
}

void DevicePanel::updateInfo() {
    QLayoutItem* i = ui->buttonLayout->takeAt(0);
    while (i != nullptr) {
        if (i->widget() != nullptr) {
            i->widget()->deleteLater();
        }
        delete i;
        i = ui->buttonLayout->takeAt(0);
    }

    NmDeviceState state = static_cast<NmDeviceState>(d->deviceInterface->property("State").toInt());
    QStringList chunkText;

    switch (d->deviceInterface->property("DeviceType").toInt()) {
        case Ethernet: { //Ethernet
            d->type = Wired;
            if (state == Disconnected || state == Failed || state == Unavailable) {
                d->chunkIcon = QIcon::fromTheme("network-wired-unavailable");
                chunkText.append(tr("Disconnected"));
                ui->connectionNameLabel->setText(tr("Wired Connection"));
                ui->connectionNameSubLabel->setText(tr("Disconnected"));

                if (state == Unavailable) {
                    QLabel* label = new QLabel();
                    label->setText(tr("To connect to this network, try plugging a cable in."));
                    ui->buttonLayout->addWidget(label);
                } else {
                    QPushButton* networksButton = new QPushButton();
                    networksButton->setText(tr("Connect"));
                    networksButton->setIcon(QIcon::fromTheme("network-connect"));
                    connect(networksButton, &QPushButton::clicked, [=] {
                        d->nmInterface->call(QDBus::NoBlock, "ActivateConnection", QVariant::fromValue(QDBusObjectPath("/")), QVariant::fromValue(d->device), QVariant::fromValue(QDBusObjectPath("/")));
                    });
                    ui->buttonLayout->addWidget(networksButton);
                }
            } else {
                d->chunkIcon = QIcon::fromTheme("network-wired-activated");
                chunkText.append(tr("Wired"));
                ui->connectionNameLabel->setText(tr("Wired Connection"));
                ui->connectionNameSubLabel->setText(tr("Connected"));

                QPushButton* disconnectButton = new QPushButton();
                disconnectButton->setText(tr("Disconnect"));
                disconnectButton->setIcon(QIcon::fromTheme("network-disconnect"));
                disconnectButton->setProperty("type", "destructive");
                connect(disconnectButton, &QPushButton::clicked, [=] {
                    QDBusInterface deviceInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
                    deviceInterface.call(QDBus::NoBlock, "Disconnect");
                });
                ui->buttonLayout->addWidget(disconnectButton);
            }
            break;
        }

        case Wifi: {
            d->type = Wifi;
            QDBusInterface wirelessInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());
            QDBusObjectPath activeNetwork = wirelessInterface.property("ActiveAccessPoint").value<QDBusObjectPath>();

            if (state == Disconnected || state == Failed || state == Unavailable || activeNetwork.path() == "/" || activeNetwork.path() == "") {
                d->chunkIcon = QIcon::fromTheme("network-wireless-disconnected");
                chunkText.append(tr("Disconnected"));
                ui->connectionNameLabel->setText(tr("Wi-Fi"));

                if (state == Unavailable) {
                    ui->connectionNameSubLabel->setText(tr("Disabled"));

                    QLabel* label = new QLabel();
                    label->setText(tr("To connect to a network, try switching on Wi-Fi."));
                    ui->buttonLayout->addWidget(label);

                } else {
                    ui->connectionNameSubLabel->setText(tr("Disconnected"));
                }
            } else {
                QDBusInterface activeNetworkInterface(d->nmInterface->service(), activeNetwork.path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus());

                int strength = activeNetworkInterface.property("Strength").toInt();
                if (strength < 15) {
                    d->chunkIcon = QIcon::fromTheme("network-wireless-connected-00");
                } else if (strength < 35) {
                    d->chunkIcon = QIcon::fromTheme("network-wireless-connected-25");
                } else if (strength < 65) {
                    d->chunkIcon = QIcon::fromTheme("network-wireless-connected-50");
                } else if (strength < 85) {
                    d->chunkIcon = QIcon::fromTheme("network-wireless-connected-75");
                } else {
                    d->chunkIcon = QIcon::fromTheme("network-wireless-connected-100");
                }

                QString ssid = activeNetworkInterface.property("Ssid").toString();
                ui->connectionNameLabel->setText(ssid);
                chunkText.append(ssid);

                if (state == Activated) {
                    ui->connectionNameSubLabel->setText(tr("Connected"));
                } else if (state == Prepare || state == Config || state == IpConfig || state == IpCheck || state == Secondaries) {
                    ui->connectionNameSubLabel->setText(tr("Connecting"));
                } else if (state == NeedAuth) {
                    ui->connectionNameSubLabel->setText(tr("Requires Attention"));
                } else if (state == Deactivating) {
                    ui->connectionNameSubLabel->setText(tr("Disconnecting"));
                }

                QPushButton* disconnectButton = new QPushButton();
                disconnectButton->setText(tr("Disconnect"));
                disconnectButton->setIcon(QIcon::fromTheme("network-disconnect"));
                disconnectButton->setProperty("type", "destructive");
                connect(disconnectButton, &QPushButton::clicked, [=] {
                    QDBusInterface deviceInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
                    deviceInterface.call(QDBus::NoBlock, "Disconnect");
                });
                ui->buttonLayout->addWidget(disconnectButton);
            }

            if (state != Unavailable) {
                QPushButton* networksButton = new QPushButton();
                networksButton->setText(tr("Choose Network"));
                networksButton->setIcon(QIcon::fromTheme("go-next"));
                connect(networksButton, &QPushButton::clicked, [=] {
                    emit connectToWirelessDevice(d->device);
                });
                ui->buttonLayout->addWidget(networksButton);

                QPushButton* settingsButton = new QPushButton();
                settingsButton->setText(tr("WiFi Settings"));
                settingsButton->setIcon(QIcon::fromTheme("configure"));
                connect(settingsButton, &QPushButton::clicked, [=] {
                    DeviceSettings* devSettings = new DeviceSettings(d->device);
                    tPopover* popover = new tPopover(devSettings);
                    popover->setDismissable(false);
                    popover->setPopoverWidth(SC_DPI(-100));
                    connect(devSettings, &DeviceSettings::done, popover, &tPopover::dismiss);
                    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
                    connect(popover, &tPopover::dismissed, devSettings, &DeviceSettings::deleteLater);
                    popover->show(d->popoverOn);
                });
                ui->buttonLayout->addWidget(settingsButton);
            }
            break;
        }

        case Bluetooth: {
            d->type = Bluetooth;
            QDBusInterface btInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device.Bluetooth", QDBusConnection::systemBus());
            d->chunkIcon = QIcon::fromTheme("network-bluetooth");
            QString interfaceName = btInterface.property("Name").toString();
            ui->connectionNameLabel->setText(interfaceName);

            if (state == Disconnected || state == Failed) {
                chunkText.append(tr("Disconnected"));
                ui->connectionNameSubLabel->setText(tr("Disconnected"));

                QPushButton* networksButton = new QPushButton();
                networksButton->setText(tr("Connect"));
                networksButton->setIcon(QIcon::fromTheme("network-connect"));
                connect(networksButton, &QPushButton::clicked, [=] {
                    d->nmInterface->call("ActivateConnection", QVariant::fromValue(QDBusObjectPath("/")), QVariant::fromValue(d->device), QVariant::fromValue(QDBusObjectPath("/")));
                });
                ui->buttonLayout->addWidget(networksButton);
            } else if (state == Unavailable) {
                chunkText.append(tr("Unavailable"));
                ui->connectionNameSubLabel->setText(tr("Unavailable"));
            } else if (state == Prepare || state == Config || state == IpConfig || state == IpCheck || state == Secondaries) {
                chunkText.append(tr("Connecting"));
                ui->connectionNameSubLabel->setText(tr("Connecting"));

                QPushButton* disconnectButton = new QPushButton();
                disconnectButton->setText(tr("Disconnect"));
                disconnectButton->setIcon(QIcon::fromTheme("network-disconnect"));
                disconnectButton->setProperty("type", "destructive");
                connect(disconnectButton, &QPushButton::clicked, [=] {
                    QDBusInterface deviceInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
                    deviceInterface.call(QDBus::NoBlock, "Disconnect");
                });
                ui->buttonLayout->addWidget(disconnectButton);
            } else {
                chunkText.append(interfaceName);
                ui->connectionNameSubLabel->setText(tr("Connected"));

                QPushButton* disconnectButton = new QPushButton();
                disconnectButton->setText(tr("Disconnect"));
                disconnectButton->setIcon(QIcon::fromTheme("network-disconnect"));
                disconnectButton->setProperty("type", "destructive");
                connect(disconnectButton, &QPushButton::clicked, [=] {
                    QDBusInterface deviceInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
                    deviceInterface.call(QDBus::NoBlock, "Disconnect");
                });
                ui->buttonLayout->addWidget(disconnectButton);
            }
            break;
        }

        case Modem: {
            QDBusInterface mInterface(d->nmInterface->service(), d->device.path(), "org.freedesktop.NetworkManager.Device.Modem", QDBusConnection::systemBus());
            uint caps = mInterface.property("CurrentCapabilities").toUInt();
            if (caps & CdmaEvdo || caps & GsmUmts || caps & Lte) {
                d->type = Cellular;
                d->chunkIcon = QIcon::fromTheme("network-cellular");

                //Get ModemManager path
                QString mmDevicePath = d->deviceInterface->property("Udi").toString();
                ModemManager::Modem modem(mmDevicePath);
                ModemManager::Modem3gpp ppp(mmDevicePath);
                ModemManager::Sim sim(modem.simPath());

                //Connect ModemManager signals
                if (!d->modemConnected) {
                    QDBusConnection::systemBus().connect("org.freedesktop.ModemManager1", mmDevicePath, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateInfo()));
                    d->modemConnected = true;
                }

                QString operatorName = sim.operatorName();
                if (operatorName == "") operatorName = ppp.operatorName();
                if (operatorName == "") operatorName = tr("Cellular");
                ui->connectionNameLabel->setText(operatorName);

                if (modem.signalQuality().signal < 15) {
                    d->chunkIcon = QIcon::fromTheme("network-cellular-connected-00");
                } else if (modem.signalQuality().signal < 35) {
                    d->chunkIcon = QIcon::fromTheme("network-cellular-connected-25");
                } else if (modem.signalQuality().signal < 65) {
                    d->chunkIcon = QIcon::fromTheme("network-cellular-connected-50");
                } else if (modem.signalQuality().signal < 85) {
                    d->chunkIcon = QIcon::fromTheme("network-cellular-connected-75");
                } else {
                    d->chunkIcon = QIcon::fromTheme("network-cellular-connected-100");
                }

                //Get PIN retries remaining
                QDBusMessage retriesMessage = QDBusMessage::createMethodCall("org.freedesktop.ModemManager1", mmDevicePath, "org.freedesktop.DBus.Properties", "Get");
                retriesMessage.setArguments({"org.freedesktop.ModemManager1.Modem", "UnlockRetries"});
                QDBusMessage retriesReplyMessage = QDBusConnection::systemBus().call(retriesMessage);

                QDBusArgument retriesArg = retriesReplyMessage.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
                QMap<uint, uint> unlockRetries;

                retriesArg >> unlockRetries;

                bool displaySettingsButton = true;

                QString registrationStateText;
                switch (ppp.registrationState()) {
                    case MM_MODEM_3GPP_REGISTRATION_STATE_ROAMING:
                    case MM_MODEM_3GPP_REGISTRATION_STATE_ROAMING_SMS_ONLY:
                    case MM_MODEM_3GPP_REGISTRATION_STATE_ROAMING_CSFB_NOT_PREFERRED:
                        registrationStateText = tr("Roaming");
                        break;
                    case MM_MODEM_3GPP_REGISTRATION_STATE_EMERGENCY_ONLY:
                        registrationStateText = tr("Emergency Only");
                        break;
                    default: ;
                }
                bool showRegistrationText = true;

                //Report the state of the modem
                MMModemState state = modem.state();
                switch (state) {
                    case MM_MODEM_STATE_FAILED:
                        displaySettingsButton = false;
                        //Find out why the modem is failing
                        switch (modem.stateFailedReason()) {
                            case MM_MODEM_STATE_FAILED_REASON_NONE:
                            case MM_MODEM_STATE_FAILED_REASON_UNKNOWN:
                                ui->connectionNameSubLabel->setText(tr("Failed"));
                                d->chunkIcon = QIcon::fromTheme("network-cellular-disconnected");
                                chunkText.append(tr("Failed"));
                                break;
                            case MM_MODEM_STATE_FAILED_REASON_SIM_MISSING: {
                                ui->connectionNameSubLabel->setText(tr("No SIM inserted"));
                                d->chunkIcon = QIcon::fromTheme("sim-card-none");
                                chunkText.append(tr("No SIM inserted"));

                                QLabel* label = new QLabel();
                                label->setText(tr("To connect to this network, try inserting a SIM card."));
                                ui->buttonLayout->addWidget(label);
                                break;
                            }
                            case MM_MODEM_STATE_FAILED_REASON_SIM_ERROR: {
                                //See if the SIM is PUK blocked
                                if (unlockRetries.value(MM_MODEM_LOCK_SIM_PUK) == 0 || unlockRetries.value(MM_MODEM_LOCK_SIM_PUK2) == 0) {
                                    ui->connectionNameSubLabel->setText(tr("PUK Blocked"));
                                    d->chunkIcon = QIcon::fromTheme("sim-card-none");
                                    chunkText.append(tr("PUK Blocked"));

                                    QLabel* label = new QLabel();
                                    label->setText(tr("Contact your mobile operator and acquire a new SIM card."));
                                    ui->buttonLayout->addWidget(label);
                                } else {
                                    ui->connectionNameSubLabel->setText(tr("SIM Error"));
                                    d->chunkIcon = QIcon::fromTheme("sim-card-none");
                                    chunkText.append(tr("SIM Error"));
                                }

                                break;
                            }
                        }
                        break;
                    case MM_MODEM_STATE_INITIALIZING:
                    case MM_MODEM_STATE_LOCKED: {
                        //Find out why the device is locked
                        //See if the SIM is PUK blocked
                        if (unlockRetries.value(MM_MODEM_LOCK_SIM_PUK) == 0 || unlockRetries.value(MM_MODEM_LOCK_SIM_PUK2) == 0) {
                            //The SIM is PUK blocked
                            ui->connectionNameSubLabel->setText(tr("PUK Blocked"));
                            d->chunkIcon = QIcon::fromTheme("sim-card-none");
                            chunkText.append(tr("PUK Blocked"));

                            QLabel* label = new QLabel();
                            label->setText(tr("Contact your mobile operator and acquire a new SIM card."));
                            ui->buttonLayout->addWidget(label);

                            displaySettingsButton = false;
                        } else {
                            MMModemLock lockType = modem.unlockRequired();
                            d->chunkIcon = QIcon::fromTheme("sim-card");

                            QPushButton* unlockButton = new QPushButton();
                            unlockButton->setIcon(QIcon::fromTheme("go-next"));
                            connect(unlockButton, &QPushButton::clicked, [=] {
                                SimPinRequest* pinRequestPopover = new SimPinRequest(mmDevicePath);
                                tPopover* popover = new tPopover(pinRequestPopover);
                                popover->setDismissable(false);
                                popover->setPopoverWidth(SC_DPI(300));
                                connect(pinRequestPopover, &SimPinRequest::done, popover, &tPopover::dismiss);
                                connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
                                connect(popover, &tPopover::dismissed, pinRequestPopover, &SimPinRequest::deleteLater);
                                popover->show(d->popoverOn);
                            });

                            bool showNotification = true;
                            tNotification* notification = new tNotification();

                            chunkText.append(operatorName);
                            switch (lockType) {
                                case MM_MODEM_LOCK_NONE:
                                    //Actually we're just initializing
                                    ui->connectionNameSubLabel->setText(tr("Initializing"));
                                    d->chunkIcon = QIcon::fromTheme("network-cellular-disconnected");
                                    showNotification = false;
                                    break;
                                case MM_MODEM_LOCK_SIM_PIN:
                                    ui->connectionNameSubLabel->setText(tr("SIM PIN required"));
                                    unlockButton->setText(tr("Provide SIM PIN"));
                                    chunkText.append(tr("SIM PIN required"));

                                    notification->setSummary(tr("SIM PIN required"));
                                    notification->setText(tr("A %1 is required to access the cellular network.").arg(tr("SIM PIN")));
                                    break;
                                case MM_MODEM_LOCK_SIM_PUK:
                                    ui->connectionNameSubLabel->setText(tr("SIM PUK required"));
                                    unlockButton->setText(tr("Provide SIM PUK"));
                                    chunkText.append(tr("SIM PUK required"));

                                    notification->setSummary(tr("SIM PUK required"));
                                    notification->setText(tr("A %1 is required to access the cellular network.").arg(tr("SIM PUK")));
                                    break;
                                case MM_MODEM_LOCK_SIM_PIN2:
                                    ui->connectionNameSubLabel->setText(tr("SIM PIN2 required"));
                                    unlockButton->setText(tr("Provide SIM PIN2"));
                                    chunkText.append(tr("SIM PIN2 required"));

                                    notification->setSummary(tr("SIM PIN2 required"));
                                    notification->setText(tr("A %1 is required to access the cellular network.").arg(tr("SIM PIN2")));
                                    break;
                                case MM_MODEM_LOCK_SIM_PUK2:
                                    ui->connectionNameSubLabel->setText(tr("SIM PUK2 required"));
                                    unlockButton->setText(tr("Provide SIM PUK2"));
                                    chunkText.append(tr("SIM PUK2 required"));

                                    notification->setSummary(tr("SIM PUK2 required"));
                                    notification->setText(tr("A %1 is required to access the cellular network.").arg(tr("SIM PUK2")));
                                    break;
                                default:
                                    ui->connectionNameSubLabel->setText(tr("Device Locked"));
                                    showNotification = false;
                                    chunkText.append(tr("Requires Attention"));
                                    d->chunkIcon = QIcon::fromTheme("network-cellular-error");
                            }
                            unlockButton->setVisible(showNotification);
                            ui->buttonLayout->addWidget(unlockButton);

                            if (!d->promptedForPin && showNotification) {
                                d->promptedForPin = true;
                                QTimer::singleShot(0, this, [=] {
                                    notification->post();
                                });
                            } else {
                                notification->deleteLater();
                            }
                        }
                        break;
                    }
                    case MM_MODEM_STATE_DISABLED: {
                        ui->connectionNameSubLabel->setText(tr("Disabled"));
                        d->chunkIcon = QIcon::fromTheme("network-cellular-disconnected");
                        chunkText.append(tr("Disabled"));
                        displaySettingsButton = false;

                        QLabel* label = new QLabel();
                        label->setText(tr("To connect to this network, try switching on Cellular."));
                        ui->buttonLayout->addWidget(label);
                        break;
                    }
                    case MM_MODEM_STATE_ENABLING:
                        ui->connectionNameSubLabel->setText(tr("Enabling"));
                        chunkText.append(tr("Enabling"));
                        break;
                    case MM_MODEM_STATE_ENABLED:
                        ui->connectionNameSubLabel->setText(tr("No Service"));
                        chunkText.append(tr("No Service"));
                        d->chunkIcon = QIcon::fromTheme("network-cellular-disconnected");
                        break;
                    case MM_MODEM_STATE_SEARCHING:
                        ui->connectionNameSubLabel->setText(tr("Searching"));
                        chunkText.append(tr("Searching"));
                        break;
                    case MM_MODEM_STATE_REGISTERED: {
                        chunkText.append(operatorName + " · " + tr("Requires Attention"));
                        ui->connectionNameSubLabel->setText(tr("Connected") + " · " + tr("No Data"));

                        QLabel* label = new QLabel();
                        label->setText(tr("Setup may be required."));
                        label->setMargin(6);
                        ui->buttonLayout->addWidget(label);
                        break;
                    }
                    case MM_MODEM_STATE_DISCONNECTING:
                        ui->connectionNameSubLabel->setText(tr("Disconnecting"));
                        chunkText.append(tr("Disconnecting"));
                        break;
                    case MM_MODEM_STATE_CONNECTING:
                        ui->connectionNameSubLabel->setText(tr("Connecting"));
                        chunkText.append(operatorName + " · " + tr("Connecting"));
                        break;
                    case MM_MODEM_STATE_CONNECTED: {
                        //Determine the type of access technology this modem is using
                        chunkText.append(operatorName);
                        QString accessTech;
                        ModemManager::Modem::AccessTechnologies techFlags = modem.accessTechnologies();
                        if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_LTE) {
                            accessTech = "4G";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_EVDOB
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_EVDOA
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_EVDO0) {
                            accessTech = "3GEVDO";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_1XRTT) {
                            accessTech = "2GXRTT";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_HSPA_PLUS) {
                            accessTech = "H+";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_HSPA
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_HSUPA
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_HSDPA) {
                            accessTech = "H";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_UMTS) {
                            accessTech = "3G";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_EDGE) {
                            accessTech = "E";
                        } else if (techFlags & MM_MODEM_ACCESS_TECHNOLOGY_GPRS
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_GSM_COMPACT
                                   || techFlags & MM_MODEM_ACCESS_TECHNOLOGY_GSM) {
                            accessTech = "G";
                        }

                        if (registrationStateText == tr("Roaming")) {
                            showRegistrationText = false;
                            accessTech += " R";
                        }

                        ui->connectionNameSubLabel->setText(tr("Connected") + " · " + accessTech);
                        chunkText.append(accessTech);
                    }
                }

                if (showRegistrationText && registrationStateText != "") {
                    ui->connectionNameSubLabel->setText(ui->connectionNameSubLabel->text() + " · " + registrationStateText);
                }

                if (displaySettingsButton) {
                    QPushButton* settingsButton = new QPushButton();
                    settingsButton->setText(tr("Cellular Settings"));
                    settingsButton->setIcon(QIcon::fromTheme("configure"));
                    connect(settingsButton, &QPushButton::clicked, [=] {
                        DeviceSettings* devSettings = new DeviceSettings(d->device);
                        tPopover* popover = new tPopover(devSettings);
                        popover->setDismissable(false);
                        popover->setPopoverWidth(SC_DPI(-100));
                        connect(devSettings, &DeviceSettings::done, popover, &tPopover::dismiss);
                        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
                        connect(popover, &tPopover::dismissed, devSettings, &DeviceSettings::deleteLater);
                        popover->show(d->popoverOn);
                    });
                    ui->buttonLayout->addWidget(settingsButton);
                }
            }
            break;
        }

        default:
            this->setVisible(false);
    }

    d->chunkText = chunkText.join(" · ");
    ui->iconLabel->setPixmap(d->chunkIcon.pixmap(SC_DPI(32), SC_DPI(32)));
    emit updated();
}

void DevicePanel::on_infoButton_clicked()
{
    emit getInformationAboutDevice(d->device);
}

QIcon DevicePanel::chunkIcon() {
    return d->chunkIcon;
}

QString DevicePanel::chunkText() {
    return d->chunkText;
}
