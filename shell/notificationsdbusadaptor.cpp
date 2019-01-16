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
#include "notificationsdbusadaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>

NotificationsDBusAdaptor* NotificationsDBusAdaptor::i = nullptr;

NotificationsDBusAdaptor::NotificationsDBusAdaptor(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "ActionInvoked", this, SLOT(actionInvokedFromDBus(uint,QString)));
}

void NotificationsDBusAdaptor::actionInvokedFromDBus(uint id, QString action_key) {
    emit ActionInvoked(id, action_key);
}

void NotificationsDBusAdaptor::CloseNotification(uint id) {
    NotificationsDBusAdaptor::instance()->DoCloseNotification(id);
}

void NotificationsDBusAdaptor::DoCloseNotification(uint id) {
    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "CloseNotification");
    msg.setArguments({id});
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

tPromise<uint>* NotificationsDBusAdaptor::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout) {
    return NotificationsDBusAdaptor::instance()->DoNotify(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout);
}

tPromise<uint>* NotificationsDBusAdaptor::DoNotify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout) {
    return new tPromise<uint>([=](QString& error) {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
        msg.setArguments({app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout});
        return QDBusConnection::sessionBus().call(msg).arguments().first().toUInt();
    });
}

NotificationsDBusAdaptor* NotificationsDBusAdaptor::instance() {
    if (NotificationsDBusAdaptor::i == nullptr) {
        NotificationsDBusAdaptor::i = new NotificationsDBusAdaptor();
    }
    return NotificationsDBusAdaptor::i;
}
