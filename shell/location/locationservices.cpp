/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#include "locationservices.h"
#include "agent_adaptor.h"

LocationServices::LocationServices(QObject *parent) : QObject(parent)
{
    QDBusMessage activator = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "StartServiceByName");
    activator.setArguments(QVariantList() << "org.freedesktop.GeoClue2");
    QDBusConnection::systemBus().call(activator);

    new AgentAdaptor(this);
    QDBusConnection dbus = QDBusConnection::systemBus();
    dbus.registerObject("/org/freedesktop/GeoClue2/Agent", "org.freedesktop.GeoClue2.Agent", this);

    QDBusMessage agentMessage = QDBusMessage::createMethodCall("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.GeoClue2.Manager", "AddAgent");
    agentMessage.setArguments(QVariantList() << "theshell");
    QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(agentMessage);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(message);
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        qDebug() << message.error().name();
        qDebug() << message.error().message();

        if (message.isError()) {
            if (message.error().name() == "org.freedesktop.DBus.Error.AccessDenied") {
                reqAuth = true;
            }
        }
        watcher->deleteLater();
    });

    QDBusConnection::systemBus().connect("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(GeocluePropertiesChanged(QString,QVariantMap)));
}

bool LocationServices::AuthorizeApp(QString desktop_id, uint req_accuracy_level, uint &allowed_accuracy_level) {
    qDebug() << desktop_id + " is accessing location";
    allowed_accuracy_level = req_accuracy_level;

    if (desktop_id == "theshell") {
        //Allow theShell
        return true;
    }



    allowed_accuracy_level = req_accuracy_level;
    return true;
}

uint LocationServices::MaxAccuracyLevel() {
    return 6;
}

void LocationServices::GeocluePropertiesChanged(QString interface, QVariantMap properties) {
    if (interface == "org.freedesktop.GeoClue2.Manager") {
        if (properties.keys().contains("InUse")) {
            emit locationUsingChanged(properties.value("InUse").toBool());
        }
    }
}

bool LocationServices::requiresAuthorization() {
    return reqAuth;
}
