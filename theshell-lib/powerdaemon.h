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
#ifndef POWERDAEMON_H
#define POWERDAEMON_H

#include <QObject>

struct PowerDaemonPrivate;
class PowerDaemon : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.thesuite.Power")

    public:
        static PowerDaemon* instance();

        void setPowerStretch(bool isOn);
        Q_SCRIPTABLE bool powerStretch();

    signals:
        Q_SCRIPTABLE void powerStretchChanged(bool powerStretch);

    public slots:

    private:
        explicit PowerDaemon(QObject *parent = nullptr);
        static PowerDaemonPrivate* d;
};

#endif // POWERDAEMON_H
