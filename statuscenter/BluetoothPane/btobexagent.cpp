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

#include "btobexagent.h"

BTObexAgent::BTObexAgent(BluezQt::ObexManager *mgr, BluezQt::Manager* btMgr, QObject *parent) : BluezQt::ObexAgent(parent) {
    this->mgr = mgr;
    this->btMgr = btMgr;

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(ActionInvoked(uint,QString)));

    BluezQt::PendingCall* serviceStarter = BluezQt::ObexManager::startService();
    serviceStarter->waitForFinished();

    BluezQt::InitObexManagerJob* job = mgr->init();
    connect(job, &BluezQt::InitObexManagerJob::result, [=] {
        if (job->error() == BluezQt::InitObexManagerJob::NoError) {
            mgr->registerAgent(this);
        }
    });
    job->start();
}

QDBusObjectPath BTObexAgent::objectPath() const {
    return QDBusObjectPath("/org/thesuite/tsbt/obex");
}

void BTObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request) {
    this->currentRequest = request;
    this->currentTransfer = transfer;

    BluezQt::DevicePtr dev = btMgr->deviceForAddress(session.data()->destination());

    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QVariantMap hints;
    hints.insert("transient", true);

    QStringList actions;
    actions.append("true");
    actions.append("Accept File");
    actions.append("false");
    actions.append("Cancel");

    QList<QVariant> args;
    args << tr("Bluetooth") << notificationNumber << "preferences-system-bluetooth" << tr("Incoming File over Bluetooth") <<
                            tr("%1 wants to send the file <b>%2</b>.").arg(dev.data()->name(), transfer.data()->name()) <<
                            actions << hints << (int) 0;

    QDBusReply<uint> reply = interface.callWithArgumentList(QDBus::Block, "Notify", args);
    this->notificationNumber = reply.value();
}

void BTObexAgent::ActionInvoked(uint id, QString action) {
    if (id == this->notificationNumber) {
        if (action == "true") {
            emit newTransfer(currentTransfer);
            QString transferName = currentTransfer.data()->name();

            QString cachePath = QFile::decodeName(qgetenv("XDG_CACHE_HOME"));
            if (cachePath == "") cachePath = QDir::homePath() + "/.cache";
            cachePath += "/obexd/";

            QString temporaryPath = cachePath + transferName;
            int number = 0;
            while (QFile::exists(temporaryPath)) {
                temporaryPath = cachePath + transferName + "_" + QString::number(number);
                number++;
            }

            //Request a new KJob Server
            QDBusMessage message = QDBusMessage::createMethodCall("org.kde.JobViewServer", "/JobViewServer", "org.kde.JobViewServer", "requestView");
            message.setArguments({
                tr("Bluetooth"),
                "preferences-system-bluetooth",
                1
            });
            QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message));
            connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
                QDBusObjectPath jobviewPath = watcher->reply().arguments().first().value<QDBusObjectPath>();

                CancelWatcher* canceller = new CancelWatcher();

                QDBusInterface* interface = new QDBusInterface("org.kde.JobViewServer", jobviewPath.path(), "org.kde.JobViewV2");
                interface->call(QDBus::NoBlock, "setInfoMessage", tr("Receiving File over Bluetooth"));
                interface->call(QDBus::NoBlock, "setDescriptionField", (uint) 1, tr("File Name"), transferName);
                interface->call(QDBus::NoBlock, "setPercent", (uint) 0);
                interface->call(QDBus::NoBlock, "setTotalAmount", (qulonglong) currentTransfer->size(), "bytes");
                interface->call(QDBus::NoBlock, "setProcessedAmount", (qulonglong) 0, "bytes");
                connect(currentTransfer.data(), &BluezQt::ObexTransfer::transferredChanged, [=](quint64 bytes) {
                    interface->call(QDBus::NoBlock, "setProcessedAmount", (qulonglong) bytes, "bytes");
                    interface->call(QDBus::NoBlock, "setPercent", (uint) ((float) bytes * 100 / (float) currentTransfer->size()));
                });
                connect(currentTransfer.data(), &BluezQt::ObexTransfer::statusChanged, [=](BluezQt::ObexTransfer::Status status) {
                    if (status == BluezQt::ObexTransfer::Complete || status == BluezQt::ObexTransfer::Error) {
                        //Finish everything
                        interface->call(QDBus::NoBlock, "terminate", "");
                        canceller->deleteLater();
                    }
                });
                QDBusConnection::sessionBus().connect("org.kde.JobViewServer", jobviewPath.path(), "org.kde.JobViewV2", "cancelRequested", canceller, SLOT(cancelRequested()));
                connect(canceller, &CancelWatcher::cancelled, [=] {
                    currentTransfer->cancel();
                });

                watcher->deleteLater();
            });

            connect(currentTransfer.data(), &BluezQt::ObexTransfer::statusChanged, [=](BluezQt::ObexTransfer::Status status) {
                if (status == BluezQt::ObexTransfer::Complete) {
                    //Move the file to a good place

                    QString permanantPath = QDir::homePath() + "/Downloads/Bluetooth/" + transferName;
                    int number = 0;
                    while (QFile::exists(permanantPath)) {
                        permanantPath = QDir::homePath() + "/Downloads/Bluetooth/" + transferName + " (" + QString::number(number) + ")";
                        number++;
                    }

                    QFile::rename(temporaryPath, permanantPath);
                }
            });

            currentRequest.accept(temporaryPath);
        } else {
            currentRequest.reject();
        }
    }
}

void BTObexAgent::cancel() {
    QDBusInterface interface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    interface.call("CloseNotification", this->notificationNumber);
}
