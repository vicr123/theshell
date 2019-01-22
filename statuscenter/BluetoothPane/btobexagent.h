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

#ifndef BTOBEXAGENT_H
#define BTOBEXAGENT_H

#include <BluezQt/ObexAgent>
#include <BluezQt/ObexManager>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/PendingCall>
#include <BluezQt/ObexTransfer>
#include <BluezQt/ObexSession>
#include <BluezQt/Request>
#include <BluezQt/Manager>
#include <BluezQt/Device>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDir>

class CancelWatcher : public QObject
{
    Q_OBJECT
    public:
        CancelWatcher(QObject* parent = nullptr) : QObject(parent) {};

    signals:
        void cancelled();

    public slots:
        void cancelRequested() {
            emit cancelled();
        }
};

class BTObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT

    public:
        explicit BTObexAgent(BluezQt::ObexManager* mgr, BluezQt::Manager* btMgr, QObject* parent = 0);

        QDBusObjectPath objectPath() const;

        void authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);
        void cancel();

    signals:
        void newTransfer(BluezQt::ObexTransferPtr transfer);

    private slots:
        void ActionInvoked(uint id, QString action);

    private:
        BluezQt::ObexManager* mgr;
        BluezQt::Manager* btMgr;

        BluezQt::Request<QString> currentRequest;
        BluezQt::ObexTransferPtr currentTransfer;
        uint notificationNumber;
};

#endif // BTOBEXAGENT_H
