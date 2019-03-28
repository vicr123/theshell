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
#include "redshiftengine.h"

#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QProcess>

struct RedshiftEnginePrivate {
    QTimer* eventTimer;
    QSettings settings;

    int overrideRedshift = 0;
    int currentRedshiftIntensity = 6500;

    bool isRedshiftOn = false;
    bool effectiveRedshiftOn = false;
    bool previewing = false;
};

RedshiftEngine::RedshiftEngine(QObject *parent) : QObject(parent)
{
    d = new RedshiftEnginePrivate();

    d->eventTimer = new QTimer(this);
    d->eventTimer->setInterval(1000);
    connect(d->eventTimer, &QTimer::timeout, this, &RedshiftEngine::processTimer);
    d->eventTimer->start();
}

RedshiftEngine::~RedshiftEngine() {
    delete d;
}

void RedshiftEngine::processTimer() {
    if (d->previewing) return; //Don't mess with Redshift during a preview

    QTime time = QTime::currentTime();
    int currentMsecs = time.msecsSinceStartOfDay();
    int startMsecs = QTime::fromString(d->settings.value("display/redshiftStart").toString()).msecsSinceStartOfDay();
    int endMsecs = QTime::fromString(d->settings.value("display/redshiftEnd").toString()).msecsSinceStartOfDay();
    int endIntensity = d->settings.value("display/redshiftIntensity", 5000).toInt();
    const int oneHour = 3600000;
    int newRedshiftIntensity;
    //QString redshiftCommand;

    //Check if Redshift is scheduled
    if (!d->settings.value("display/redshiftPaused", true).toBool()) {
        //Redshift is scheduled
        //Calculate redshift value
        //Transition to redshift is 1 hour from the start.

        int intensity;
        if (startMsecs > endMsecs) { //Start time is later then end time
            if (currentMsecs < endMsecs || currentMsecs > startMsecs) {
                intensity = endIntensity;
            } else if (currentMsecs < startMsecs && currentMsecs > startMsecs - oneHour) {
                int timeFrom = currentMsecs - (startMsecs - oneHour);
                float percentage = ((float) timeFrom / (float) oneHour);
                int progress = (6500 - endIntensity) * percentage;
                intensity = 6500 - progress;
            } else if (currentMsecs > endMsecs && currentMsecs < endMsecs + oneHour) {
                int timeFrom = endMsecs - (currentMsecs - oneHour);
                float percentage = ((float) timeFrom / (float) oneHour);
                int progress = (6500 - endIntensity) * percentage;
                intensity = 6500 - progress;
            } else {
                intensity = 6500;
            }
        } else { //Start time is earlier then end time
            if (currentMsecs < endMsecs && currentMsecs > startMsecs) {
                intensity = endIntensity;
            } else if (currentMsecs < startMsecs && currentMsecs > startMsecs - oneHour) {
                int timeFrom = currentMsecs - (startMsecs - oneHour);
                float percentage = ((float) timeFrom / (float) oneHour);
                int progress = (6500 - endIntensity) * percentage;
                intensity = 6500 - progress;
            } else if (currentMsecs > endMsecs && currentMsecs < endMsecs + oneHour) {
                int timeFrom = endMsecs - (currentMsecs - oneHour);
                float percentage = ((float) timeFrom / (float) oneHour);
                int progress = (6500 - endIntensity) * percentage;
                intensity = 6500 - progress;
            } else {
                intensity = 6500;
            }
        }

        //Check Redshift override
        if (d->overrideRedshift != 0) {
            if (intensity == 6500 && d->overrideRedshift == 1) {
                d->overrideRedshift = 0; //Reset Redshift override
            } else if (intensity != 6500 && d->overrideRedshift == 2) {
                d->overrideRedshift = 0; //Reset Redshift override
            } else {
                if (d->overrideRedshift == 1) {
                    intensity = 6500;
                } else {
                    intensity = endIntensity;
                }
            }
        }

        newRedshiftIntensity = intensity;

        d->isRedshiftOn = true;
        if (intensity == 6500 && d->effectiveRedshiftOn) {
            d->effectiveRedshiftOn = false;
            emit redshiftEnabledChanged(false);
        } else if (intensity != 6500 && !d->effectiveRedshiftOn) {
            d->effectiveRedshiftOn = true;
            emit redshiftEnabledChanged(true);
        }
    } else {
        //Redshift is not scheduled
        //Check Redshift Override
        if (d->overrideRedshift == 2) {
            newRedshiftIntensity = endIntensity;
        } else {
            newRedshiftIntensity = 6500;
        }

        if (d->isRedshiftOn) {
            d->isRedshiftOn = false;
            d->effectiveRedshiftOn = false;
            emit redshiftEnabledChanged(false);
        }
    }

    if (d->currentRedshiftIntensity != newRedshiftIntensity) {
        adjust(newRedshiftIntensity);
    }
}

void RedshiftEngine::preview(int temp) {
    d->previewing = true;
    adjust(temp);
}

void RedshiftEngine::endPreview() {
    d->previewing = false;
    processTimer();
}

void RedshiftEngine::adjust(int temp) {
    QProcess* redshiftAdjust = new QProcess();
    redshiftAdjust->start("redshift -P -O " + QString::number(temp));
    connect(redshiftAdjust, SIGNAL(finished(int)), redshiftAdjust, SLOT(deleteLater()));
    d->currentRedshiftIntensity = temp;
}

void RedshiftEngine::overrideRedshift(bool enabled) {
    if (d->effectiveRedshiftOn) {
        if (enabled) { //Turn Redshift back on
            d->overrideRedshift = 0;
        } else { //Temporarily disable Redshift
            d->overrideRedshift = 1;
        }
    } else {
        if (enabled) { //Temporarily enable Redshift
            d->overrideRedshift = 2;
        } else { //Turn Redshift back off
            d->overrideRedshift = 0;
        }
    }
}

bool RedshiftEngine::isEnabled() {
    return d->effectiveRedshiftOn;
}
