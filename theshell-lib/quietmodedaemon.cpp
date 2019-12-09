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
#include "quietmodedaemon.h"

#include <QDateTime>
#include <QTimer>

struct QuietModeDaemonPrivate {
    QuietModeDaemon* instance = nullptr;

    QuietModeDaemon::QuietMode quietMode = QuietModeDaemon::None;
    QTimer* quietModeWatcher;
    QDateTime quietModeOff;
};

QuietModeDaemonPrivate* QuietModeDaemon::d = new QuietModeDaemonPrivate();

QuietModeDaemon::QuietModeDaemon(QObject *parent) : QObject(parent)
{
    d->quietModeWatcher = new QTimer();
    d->quietModeWatcher->setInterval(1000);
    connect(d->quietModeWatcher, &QTimer::timeout, [=] {
        if (QDateTime::currentDateTime() > d->quietModeOff) {
            setQuietMode(None);
            d->quietModeWatcher->stop();
        }
    });
}

QuietModeDaemon*QuietModeDaemon::instance()
{
    if (!d->instance) d->instance = new QuietModeDaemon();
    return d->instance;
}

QuietModeDaemon::QuietMode QuietModeDaemon::getQuietMode()
{
    return d->quietMode;
}

void QuietModeDaemon::setQuietMode(QuietModeDaemon::QuietMode quietMode)
{
    if (quietMode != d->quietMode) {
        QuietMode oldQuietMode = d->quietMode;
        d->quietMode = quietMode;
        if (d->instance) emit d->instance->QuietModeChanged(quietMode, oldQuietMode);
    }
}

void QuietModeDaemon::setQuietModeResetTime(QDateTime time)
{
    if (time.isValid()) {
        d->quietModeOff = time;
        d->quietModeWatcher->start();
    } else {
        d->quietModeWatcher->stop();
    }
}

QString QuietModeDaemon::getCurrentQuietModeDescription()
{
    switch (d->quietMode) {
        case None:
            return tr("Allows all sounds from all apps, and notifications from all apps.");
        case Critical:
            return tr("Ignores all notifications not marked as critical and those set to bypass Quiet Mode. Normal sounds will still be played.");
        case Notifications:
            return tr("Ignores any notifications from all apps, except those set to bypass Quiet Mode. Normal sounds will still be played, and timers and reminders will still notify you, however, they won't play sounds.");
        case Mute:
            return tr("Completely turns off all sounds and notifications from all apps, including those set to bypass Quiet Mode. Not even timers or reminders will notify you.");
    }
}
