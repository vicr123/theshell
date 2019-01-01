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

#include "polkitinterface.h"

PolkitInterface::PolkitInterface(QObject *parent) : PolkitQt1::Agent::Listener(parent)
{
    //Register our Polkit service on DBus
    new PolkitAuthAgentAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.thesuite.polkitAuthAgent");

    if (!QDBusConnection::sessionBus().registerObject("/org/thesuite/polkitAuthAgent", this,
                                                      QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableProperties | QDBusConnection::ExportAdaptors)) {
        qDebug() << "Could not initiate Authentication Agent on DBus!";
    }

    //Create a new authentication window
    authWin = new Authenticate();
    connect(authWin, SIGNAL(okClicked()), this, SLOT(windowAccepted()));
    connect(authWin, SIGNAL(rejected()), this, SLOT(windowRejected()));
    connect(authWin, SIGNAL(newUser(PolkitQt1::Identity)), this, SLOT(setUser(PolkitQt1::Identity)));
}

void PolkitInterface::windowAccepted() { //User clicked OK
    //Initialize the session.
    this->initSession();
    this->session->setResponse(authWin->getPassword());
}

void PolkitInterface::windowRejected() { //User clicked Cancel
    this->initSession();
    this->dialogCanceled = true;
    this->session->cancel();
}

void PolkitInterface::initiateAuthentication(const QString &actionId, const QString &message, const QString &iconName, const PolkitQt1::Details &details, const QString &cookie, const PolkitQt1::Identity::List &identities, PolkitQt1::Agent::AsyncResult *result) {
    //New Polkit Authentication Request. Set required variables.
    currentIdentity = identities.first();
    this->asyncResult = result;
    this->cookie = cookie;

    //Show Authentication Window.
    authWin->setMessage(message);
    authWin->setDetails(details);
    authWin->setIcon(QIcon::fromTheme(iconName, QIcon::fromTheme("dialog-password")));
    authWin->setUsers(identities);
    authWin->setUser(identities.first());
    authWin->showFullScreen(false);

    isAuthenticating = true;
}

void PolkitInterface::initSession() {
    this->dialogCanceled = false;
    this->session = new PolkitQt1::Agent::Session(currentIdentity, cookie, asyncResult, this);
    connect(session, SIGNAL(request(QString,bool)), this, SLOT(sessionRequest(QString,bool)));
    connect(session, SIGNAL(completed(bool)), this, SLOT(sessionComplete(bool)));
    session->initiate();
}

void PolkitInterface::sessionComplete(bool ok) {
    this->authenticatedOk = ok;
    this->finish();
}

void PolkitInterface::finish() {
    if (authenticatedOk || dialogCanceled) {
        if (session == nullptr) {
            this->asyncResult->setCompleted();
        } else {
            session->result()->setCompleted();
        }
        session->deleteLater();
        authWin->close();
        isAuthenticating = false;
    } else {
        session->deleteLater();
        //initSession();
        isAuthenticating = true;
        authWin->showFullScreen(true);
    }
    session = nullptr;
}

void PolkitInterface::sessionRequest(QString request, bool echo) {
    if (request.startsWith("password:", Qt::CaseInsensitive)) {
        authWin->setUser(currentIdentity.toString().remove("unix-user:"));
    }
}

bool PolkitInterface::initiateAuthenticationFinish() {
    return true;
}

void PolkitInterface::cancelAuthentication() {
    authWin->close();
    isAuthenticating = false;
}

void PolkitInterface::setUser(PolkitQt1::Identity newUser) {
    this->currentIdentity = newUser;
}
