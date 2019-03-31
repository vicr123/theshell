/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef NOTIFICATIONSPERMISSIONENGINE_H
#define NOTIFICATIONSPERMISSIONENGINE_H

#include <QString>
#include <QIcon>

struct NotificationsPermissionEnginePrivate;
class NotificationsPermissionEngine
{
    public:
        NotificationsPermissionEngine(QString appName, QString desktopFile = "");
        NotificationsPermissionEngine();
        ~NotificationsPermissionEngine();

        static QStringList knownApps();
        static QStringList knownDesktopFiles();

        bool isValidApp();
        bool isDesktopFile();
        QString identifier();
        QIcon appIcon();
        QString appName();
        void remove();

        void setAllowNotifications(bool allow);
        bool allowNotifications();

        void setBypassesQuietMode(bool bypassesQuietMode);
        bool bypassesQuietMode();

        void setShowPopups(bool popups);
        bool showPopups();

        void setPlaySound(bool playSound);
        bool playSound();

    private:
        NotificationsPermissionEnginePrivate* d;
};

#endif // NOTIFICATIONSPERMISSIONENGINE_H
