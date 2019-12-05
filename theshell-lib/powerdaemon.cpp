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
#include "powerdaemon.h"

#include "power_adaptor.h"
#include <QDBusConnection>

struct PowerDaemonPrivate {
    PowerDaemon* instance = nullptr;

    bool powerStretch = false;
};

PowerDaemonPrivate* PowerDaemon::d = new PowerDaemonPrivate();

PowerDaemon::PowerDaemon(QObject *parent) : QObject(parent)
{
    new PowerAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/org/thesuite/Power", "org.thesuite.Power", this);

    QSettings settings;
    this->setPowerStretch(settings.value("powerstretch/on", false).toBool());
}

PowerDaemon*PowerDaemon::instance()
{
    if (!d->instance) d->instance = new PowerDaemon();
    return d->instance;
}

void PowerDaemon::setPowerStretch(bool isOn)
{
    if (!d->powerStretch == isOn) {
        QSettings settings;
        settings.setValue("powerstretch/on", isOn);

        d->powerStretch = isOn;
        emit powerStretchChanged(isOn);
    }
}

bool PowerDaemon::powerStretch()
{
    return d->powerStretch;
}
