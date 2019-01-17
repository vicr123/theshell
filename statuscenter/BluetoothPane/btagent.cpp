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

#include "btagent.h"

BTAgent::BTAgent(BluezQt::Manager* mgr, QObject* parent) : BluezQt::Agent(parent)
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/thesuite/tsbt", this);
    dbus.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(ActionInvoked(uint,QString)));

    this->mgr = mgr;
    BluezQt::InitManagerJob* job = mgr->init();
    connect(job, &BluezQt::InitManagerJob::result, [=] {
        BluezQt::PendingCall* agentRegister = mgr->registerAgent(this);
        connect(agentRegister, &BluezQt::PendingCall::finished, [=] {
            BluezQt::PendingCall* defaultAgent = mgr->requestDefaultAgent(this);
        });
    });
    job->start();
}

QDBusObjectPath BTAgent::objectPath() const {
    return QDBusObjectPath("/org/thesuite/tsbt");
}

void BTAgent::setPairingRequestDevice(BluezQt::DevicePtr device) {
    if (device.isNull()) {
        this->pairingDevice = "";
    } else {
        this->pairingDevice = device.data()->address();
    }
}

void BTAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) {
    if (pairingDevice == device.data()->address()) {
        emit pairRequest(pinCode, BluezQt::Request<>(), tr("Key in this code"), false);
    } else {

    }
}

void BTAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) {
    if (pairingDevice == device.data()->address()) {
        request.accept();
    } else {
        currentRequest = request;

        QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        QVariantMap hints;
        hints.insert("transient", true);

        QStringList actions;
        actions.append("true");
        actions.append("Accept Pairing");
        actions.append("false");
        actions.append("Cancel");

        QList<QVariant> args;
        args << tr("Bluetooth") << notificationNumber << "preferences-system-bluetooth" << tr("Pair Request") <<
                                tr("%1 wants to pair with your device.").arg(device.data()->name()) <<
                                actions << hints << (int) 0;

        QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
        this->notificationNumber = reply.value();
    }
}

void BTAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) {
    if (pairingDevice == device.data()->address()) {
        emit pairReturnRequest(request, tr("Enter Pairing code"));
    }
}

void BTAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) {
    if (pairingDevice == device.data()->address()) {
        emit pairRequest(passkey, BluezQt::Request<>(), tr("Key in this code"), false);
    } else {

    }
}

void BTAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request) {
    if (pairingDevice == device.data()->address()) {
        emit pairRequest(passkey, request, tr("Check this code matches"), true);
    } else {
        currentRequest = request;

        /*tNotification* n = new tNotification();
        n->setAppName(tr("Bluetooth"));
        n->setAppIcon("preferences-system-bluetooth");
        n->setSummary(tr("Pair Request"));
        n->setText(tr("%1 wants to pair with your device. Check that the code <b>%2</b> matches with the code shown on %1.").arg(device.data()->name(), passkey));*/

        QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        QVariantMap hints;
        hints.insert("transient", true);

        QStringList actions;
        actions.append("true");
        actions.append("Accept Pairing");
        actions.append("false");
        actions.append("Cancel");

        QList<QVariant> args;
        args << tr("Bluetooth") << notificationNumber << "preferences-system-bluetooth" << tr("Pair Request") <<
                                tr("%1 wants to pair with your device. Check that the code <b>%2</b> matches with the code shown on %1.").arg(device.data()->name(), passkey) <<
                                actions << hints << (int) 0;

        QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
        this->notificationNumber = reply.value();
    }
}

void BTAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) {
    request.accept();
}

void BTAgent::cancel() {
    if (!pairingDevice.isNull()) {
        emit pairCancel();
    } else {
        QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        interface.call("CloseNotification", this->notificationNumber);
    }
}

void BTAgent::ActionInvoked(uint id, QString action) {
    if (id == this->notificationNumber) {
        if (action == "true") {
            currentRequest.accept();
        } else {
            currentRequest.reject();
        }
    }
}
