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
