#ifndef BTOBEXAGENT_H
#define BTOBEXAGENT_H

#include <BluezQt/ObexAgent>
#include <BluezQt/ObexManager>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/PendingCall>
#include <QDBusObjectPath>

class BTObexAgent : public BluezQt::ObexAgent
{
    public:
        explicit BTObexAgent(BluezQt::ObexManager* mgr, QObject* parent = 0);

        QDBusObjectPath objectPath() const;
    private:
        BluezQt::ObexManager* mgr;
};

#endif // BTOBEXAGENT_H
