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

#ifndef BTAGENT_H
#define BTAGENT_H

#include <BluezQt/Agent>
#include <BluezQt/Manager>
#include <BluezQt/InitManagerJob>
#include <BluezQt/PendingCall>
#include <BluezQt/Device>
#include <BluezQt/Request>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <tnotification.h>

class BTAgent : public BluezQt::Agent
{
    Q_OBJECT

    public:
        explicit BTAgent(BluezQt::Manager* manager, QObject* parent = 0);

        QDBusObjectPath objectPath() const;
        void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode);
        void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request);
        void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request);
        void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered);
        void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request);
        void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request);
        void cancel();

        void setPairingRequestDevice(BluezQt::DevicePtr device);

    private slots:
        void ActionInvoked(uint id, QString action);

    signals:
        void pairRequest(QString pin, BluezQt::Request<> req, QString explanation, bool showContinueButton);
        void pairReturnRequest(BluezQt::Request<QString> req, QString explanation);
        void pairCancel();

    private:
        BluezQt::Manager* mgr;

        BluezQt::Request<> currentRequest;
        QString pairingDevice;
        uint notificationNumber;
};

#endif // BTAGENT_H
