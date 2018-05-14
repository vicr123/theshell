#ifndef POLKITINTERFACE_H
#define POLKITINTERFACE_H

#include <QObject>
#include <polkit-qt5-1/PolkitQt1/Agent/Listener>
#include <polkit-qt5-1/PolkitQt1/Details>
#include <polkit-qt5-1/PolkitQt1/Identity>
#include <polkit-qt5-1/PolkitQt1/Subject>
#include <QDebug>
#include <QDBusConnection>
#include <QApplication>
#include "polkitauthagent_adaptor.h"
#include "authenticate.h"

class PolkitInterface : public PolkitQt1::Agent::Listener
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.polkitAuthAgent")


public:
    explicit PolkitInterface(QObject *parent = 0);

signals:

public Q_SLOTS:
    void initiateAuthentication(const QString &actionId,
                                        const QString &message,
                                        const QString &iconName,
                                        const PolkitQt1::Details &details,
                                        const QString &cookie,
                                        const PolkitQt1::Identity::List &identities,
                                        PolkitQt1::Agent::AsyncResult *result);
    bool initiateAuthenticationFinish();
    void cancelAuthentication();

    void sessionRequest(QString request, bool echo);
    void sessionComplete(bool ok);
    void finish();
    void initSession();

    void windowAccepted();
    void windowRejected();
    void setUser(PolkitQt1::Identity newUser);

private:
    Authenticate* authWin;
    bool isAuthenticating = false;
    bool authenticatedOk;
    bool dialogCanceled = false;
    PolkitQt1::Agent::Session* session;
    QString cookie;
    PolkitQt1::Agent::AsyncResult* asyncResult;
    PolkitQt1::Identity currentIdentity;
};

//Q_DECLARE_METATYPE(PolkitQt1::Details)
//Q_DECLARE_METATYPE(PolkitQt1::Identity::List)
//Q_DECLARE_METATYPE(PolkitQt1::Agent::AsyncResult)

#endif // POLKITINTERFACE_H
