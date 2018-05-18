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

#include "notificationsdbusadaptor.h"
#include "notificationswidget.h"
#include "notificationobject.h"
#include "audiomanager.h"
#include "internationalisation.h"

extern AudioManager* AudioMan;

NotificationsDBusAdaptor::NotificationsDBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    this->setAutoRelaySignals(true);
    applicationNotifications = new QSettings("theSuite", "theShell-notifications");
}

NotificationsDBusAdaptor::~NotificationsDBusAdaptor()
{

}

void NotificationsDBusAdaptor::CloseNotification(uint id)
{
    if (this->parentWidget()->hasNotificationId(id)) {
        NotificationObject* notification = this->parentWidget()->getNotification(id);
        notification->dismiss();
    } else {

    }
}

QStringList NotificationsDBusAdaptor::GetCapabilities()
{
    return QStringList() << "body" << "body-hyperlinks" << "body-markup" << "persistence" << "sound" << "action-icons";
}

QString NotificationsDBusAdaptor::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    vendor = "theSuite";
    version = QString(TS_VERSION);
    spec_version = "1.2";
    return "theShell";
}

uint NotificationsDBusAdaptor::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout)
{
    if (this->parentWidget() != NULL) {
        QStringList knownApplications;
        int amount = applicationNotifications->beginReadArray("notifications/knownApplications");
        for (int i = 0; i < amount; i++) {
            applicationNotifications->setArrayIndex(i);
            knownApplications.append(applicationNotifications->value("appname").toString());
        }
        applicationNotifications->endArray();

        if (!knownApplications.contains(app_name)) {
            knownApplications.append(app_name);
            applicationNotifications->beginWriteArray("notifications/knownApplications");

            int i = 0;
            for (QString appname : knownApplications) {
                applicationNotifications->setArrayIndex(i);
                applicationNotifications->setValue("appname", appname);
                i++;
            }
            applicationNotifications->endArray();
        }

        if (!applicationNotifications->value(app_name + "/allow", true).toBool()) {
            //User doesn't want this app to post notifications
            emit NotificationClosed(999999, 2);
            return 999999;
        }

        NotificationObject* notification;
        if (this->parentWidget()->hasNotificationId(replaces_id)) {
            notification = this->parentWidget()->getNotification(replaces_id);
        } else {
            notification = new NotificationObject(app_name, app_icon, summary, body, actions, hints, expire_timeout);
        }
        this->parentWidget()->addNotification(notification);

        bool postNotification = true;
        if (AudioMan->QuietMode() == AudioManager::notifications && !applicationNotifications->value(app_name + "/bypassQuiet", false).toBool()) {
            QStringList allowedCategories;
            allowedCategories.append("battery.low");
            allowedCategories.append("battery.critical");
            allowedCategories.append("reminder.activate");
            if (!allowedCategories.contains(hints.value("category").toString()) && !hints.value("x-thesuite-timercomplete", false).toBool()) {
                postNotification = false;
                emit NotificationClosed(notification->getId(), NotificationObject::Undefined);
            }
        } else if (AudioMan->QuietMode() == AudioManager::mute) {
            postNotification = false;
            emit NotificationClosed(notification->getId(), NotificationObject::Undefined);
        }

        if (postNotification) {
            notification->post();
        }

        //Send the notification to the lock screen if the user desires
        if (settings.value("notifications/lockScreen").toString() != "none") {
            //If the notification is transient, don't send it to the lock screen
            if (!hints.value("transient", false).toBool()) {
                //Create a DBus message relaying the message to the lock screen
                QDBusMessage NotificationEmit = QDBusMessage::createMethodCall("org.thesuite.tsscreenlock", "/org/thesuite/tsscreenlock", "org.thesuite.tsscreenlock.Notifications", "newNotification");
                QVariantList NotificationArgs;

                if (settings.value("notifications/lockScreen", "noContents").toString() == "contents") {
                    NotificationArgs.append(summary);
                    NotificationArgs.append(body);
                    NotificationArgs.append(notification->getId());
                    NotificationArgs.append(actions);
                } else {
                    NotificationArgs.append(app_name);
                    NotificationArgs.append("New Notification");
                    NotificationArgs.append(notification->getId());
                    NotificationArgs.append(QStringList());
                }
                NotificationArgs.append(hints);

                NotificationEmit.setArguments(NotificationArgs);
                QDBusConnection::sessionBus().call(NotificationEmit, QDBus::NoBlock);
            }
        }

        return notification->getId();
    }
    return 0;
}

NotificationsWidget* NotificationsDBusAdaptor::parentWidget() {
    return pt;
}

void NotificationsDBusAdaptor::setParentWidget(NotificationsWidget *parent) {
    pt = parent;
}
