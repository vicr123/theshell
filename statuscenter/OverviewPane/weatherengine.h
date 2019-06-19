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
#ifndef WEATHERENGINE_H
#define WEATHERENGINE_H

#include <QObject>
#include <tpromise.h>
#include <QIcon>
#include <debuginformationcollector.h>

struct WeatherCondition {
    QDateTime startTime, endTime;

    enum KnownProperties {
        LowTemp,
        HighTemp,
        CurrentTemp,
        Icon,
        Condition,
        isRainy,
        isCloudy,
        WindSpeed,
        WindBeaufort
    };
    QMap<KnownProperties, QVariant> properties;

    bool isValid = false;
};

struct WeatherEnginePrivate;
class WeatherEngine : public QObject
{
        Q_OBJECT
    public:
        explicit WeatherEngine(QObject *parent = T_QOBJECT_ROOT);
        ~WeatherEngine();

        void setCoordinates(double latitude, double longitude, int altitude);

    signals:

    public slots:
        tPromise<WeatherCondition>* getCurrentWeather();
        tPromise<WeatherCondition>* getTodayWeather();

    private:
        WeatherEnginePrivate* d;

        bool updateWeather();
        WeatherCondition instantaneousWeather(QDateTime endTime);
};

#endif // WEATHERENGINE_H
