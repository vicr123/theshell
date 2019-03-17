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

#ifndef JOBDBUS_H
#define JOBDBUS_H

#include <QObject>
#include <QDBusVariant>

class NotificationsWidget;
class JobViewWidget;

class JobDBus : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.JobViewV2")

    public:
        explicit JobDBus(NotificationsWidget* widget, QString title, QString icon, QString path, int capabilities, QObject *parent = nullptr);

        QString title();
        QString description();
        uint percent();

        enum Capabilities {
            NoCapabilities = 0x0,
            Killable = 0x1,
            Suspendable = 0x2
        };

    public Q_SLOTS:
        Q_SCRIPTABLE void terminate(QString errorMessage);
        Q_SCRIPTABLE void setSuspended(bool suspended);
        Q_SCRIPTABLE void setTotalAmount(qulonglong amount, QString unit);
        Q_SCRIPTABLE void setProcessedAmount(qulonglong amount, QString unit);
        Q_SCRIPTABLE void setPercent(uint percent);
        Q_SCRIPTABLE void setSpeed(qulonglong bytesPerSecond);
        Q_SCRIPTABLE void setInfoMessage(QString message);
        Q_SCRIPTABLE void setDescriptionField(uint number, QString name, QString value);
        Q_SCRIPTABLE void clearDescriptionField(uint number);
        Q_SCRIPTABLE void setDestUrl(QDBusVariant url);
        Q_SCRIPTABLE void setError(uint errorCode);

    Q_SIGNALS:
        Q_SCRIPTABLE void suspendRequested();
        Q_SCRIPTABLE void resumeRequested();
        Q_SCRIPTABLE void cancelRequested();

        void update(QString title, QString description, uint percentage);
        void complete();

    private:
        QString path;

        QString t, d;
        uint p = 0;
        bool suspended = false;

        JobViewWidget* view;
};

#endif // JOBDBUS_H
