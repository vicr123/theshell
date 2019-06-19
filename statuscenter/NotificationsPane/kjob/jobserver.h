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

#ifndef JOBSERVER_H
#define JOBSERVER_H

#include <QObject>
#include <QDBusObjectPath>
#include <statuscenterpaneobject.h>
#include "jobdbus.h"

class NotificationsWidget;

class JobServer : public QObject, public StatusCenterPaneObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.JobViewServer")

    public:
        explicit JobServer(NotificationsWidget* widget, QObject *parent = T_QOBJECT_ROOT);

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args = QVariantList());

    public Q_SLOTS:
        Q_SCRIPTABLE QDBusObjectPath requestView(QString appName, QString appIconName, int capabilities);
        void updateJobs();

    private:
        int currentView = 0;
        QList<JobDBus*> jobs;

        NotificationsWidget* notificationsWidget;
};

#endif
