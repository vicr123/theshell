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
#ifndef NOTIFICATIONSDBUSADAPTOR_H
#define NOTIFICATIONSDBUSADAPTOR_H

#include <QObject>
#include <tpromise.h>
#include <debuginformationcollector.h>

class NotificationsDBusAdaptor : public QObject
{
        Q_OBJECT
    public:
        explicit NotificationsDBusAdaptor(QObject *parent = T_QOBJECT_ROOT);

        static NotificationsDBusAdaptor* instance();
    signals:
        void ActionInvoked(uint id, const QString &action_key);

    public slots:
        static tPromise<uint>* Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout);
        static void CloseNotification(uint id);

    private slots:
        void actionInvokedFromDBus(uint id, QString action_key);
        tPromise<uint>* DoNotify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout);
        void DoCloseNotification(uint id);

    private:
        static NotificationsDBusAdaptor* i;
};

#endif // NOTIFICATIONSDBUSADAPTOR_H
