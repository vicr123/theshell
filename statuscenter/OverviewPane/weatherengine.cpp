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
#include "weatherengine.h"
#include <locationdaemon.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>

struct WeatherEnginePrivate {
    QMutex knownConditionsMutex, updatingMutex;
    QList<WeatherCondition> knownConditions;

    QDateTime lastUpdate;

    double latitude, longitude;
    int altitude;

    static const QMap<int, const char*> condition;
};

const QMap<int, const char*> WeatherEnginePrivate::condition = {
    {1, QT_TRANSLATE_NOOP("WeatherEngine", "clear skies")},
    {2, QT_TRANSLATE_NOOP("WeatherEngine", "light clouds")},
    {3, QT_TRANSLATE_NOOP("WeatherEngine", "partial clouds")},
    {4, QT_TRANSLATE_NOOP("WeatherEngine", "clouds")},
    {5, QT_TRANSLATE_NOOP("WeatherEngine", "light rain and sun")},
    {6, QT_TRANSLATE_NOOP("WeatherEngine", "light rain, thunder and sun")},
    {7, QT_TRANSLATE_NOOP("WeatherEngine", "sleet and sun")},
    {8, QT_TRANSLATE_NOOP("WeatherEngine", "snow and sun")},
    {9, QT_TRANSLATE_NOOP("WeatherEngine", "light rain")},
    {10, QT_TRANSLATE_NOOP("WeatherEngine", "rain")},
    {11, QT_TRANSLATE_NOOP("WeatherEngine", "rain and thunder")},
    {12, QT_TRANSLATE_NOOP("WeatherEngine", "sleet")},
    {13, QT_TRANSLATE_NOOP("WeatherEngine", "snow")},
    {14, QT_TRANSLATE_NOOP("WeatherEngine", "snow and thunder")},
    {15, QT_TRANSLATE_NOOP("WeatherEngine", "fog")},
    {20, QT_TRANSLATE_NOOP("WeatherEngine", "sleet, sun and thunder")},
    {21, QT_TRANSLATE_NOOP("WeatherEngine", "snow, sun and thunder")},
    {22, QT_TRANSLATE_NOOP("WeatherEngine", "light rain and thunder")},
    {23, QT_TRANSLATE_NOOP("WeatherEngine", "sleet and thunder")},
    {24, QT_TRANSLATE_NOOP("WeatherEngine", "drizzle, thunder and sun")},
    {25, QT_TRANSLATE_NOOP("WeatherEngine", "rain, thunder and sun")},
    {26, QT_TRANSLATE_NOOP("WeatherEngine", "light sleet, thunder and sun")},
    {27, QT_TRANSLATE_NOOP("WeatherEngine", "heavy sleet, thunder and sun")},
    {28, QT_TRANSLATE_NOOP("WeatherEngine", "light snow, thunder and sun")},
    {29, QT_TRANSLATE_NOOP("WeatherEngine", "heavy snow, thunder and sun")},
    {30, QT_TRANSLATE_NOOP("WeatherEngine", "drizzle and thunder")},
    {31, QT_TRANSLATE_NOOP("WeatherEngine", "light sleet and thunder")},
    {32, QT_TRANSLATE_NOOP("WeatherEngine", "heavy sleet and thunder")},
    {33, QT_TRANSLATE_NOOP("WeatherEngine", "light snow and thunder")},
    {34, QT_TRANSLATE_NOOP("WeatherEngine", "heavy snow and thunder")},
    {40, QT_TRANSLATE_NOOP("WeatherEngine", "drizzle and sun")},
    {41, QT_TRANSLATE_NOOP("WeatherEngine", "rain and sun")},
    {42, QT_TRANSLATE_NOOP("WeatherEngine", "light sleet and sun")},
    {43, QT_TRANSLATE_NOOP("WeatherEngine", "heavy sleet and sun")},
    {44, QT_TRANSLATE_NOOP("WeatherEngine", "light snow and sun")},
    {45, QT_TRANSLATE_NOOP("WeatherEngine", "heavy snow and sun")},
    {46, QT_TRANSLATE_NOOP("WeatherEngine", "drizzle")},
    {47, QT_TRANSLATE_NOOP("WeatherEngine", "light sleet")},
    {48, QT_TRANSLATE_NOOP("WeatherEngine", "heavy sleet")},
    {49, QT_TRANSLATE_NOOP("WeatherEngine", "light snow")},
    {50, QT_TRANSLATE_NOOP("WeatherEngine", "heavy snow")}
};

WeatherEngine::WeatherEngine(QObject *parent) : QObject(parent)
{
    d = new WeatherEnginePrivate();
    d->lastUpdate = QDateTime::fromMSecsSinceEpoch(0);
}

WeatherEngine::~WeatherEngine() {
    delete d;
}

void WeatherEngine::setCoordinates(double latitude, double longitude, int altitude) {
    d->latitude = latitude;
    d->longitude = longitude;
    d->altitude = altitude;
}

tPromise<WeatherCondition>* WeatherEngine::getCurrentWeather() {
    return new tPromise<WeatherCondition>([=](QString& error) {
        if (!updateWeather()) {
            error = "Weather update issue";
            return WeatherCondition();
        }

        QDateTime now = QDateTime::currentDateTime();

        //Lock the mutex so we can read it
        QMutexLocker locker(&d->knownConditionsMutex);
        WeatherCondition condition;
        condition.isValid = true;

        QList<WeatherCondition> mergeConditions;
        for (WeatherCondition c : d->knownConditions) {
            if (c.startTime < now && c.endTime > now) {
                mergeConditions.append(c);
            }
        }
        locker.unlock();

        //Sort the merge conditions by time
        std::sort(mergeConditions.begin(), mergeConditions.end(), [=](const WeatherCondition& first, const WeatherCondition& second) {
            return first.startTime.msecsTo(first.endTime) > second.startTime.msecsTo(second.endTime);
        });

        WeatherCondition instantaneousCondition;
        for (WeatherCondition c : mergeConditions) {
            WeatherCondition newInstantaneousCondition = instantaneousWeather(c.endTime);
            if (!instantaneousCondition.isValid || abs(newInstantaneousCondition.endTime.msecsTo(now)) < instantaneousCondition.endTime.msecsTo(now)) {
                instantaneousCondition = newInstantaneousCondition;
            }

            //Fill in the fields
            for (WeatherCondition::KnownProperties property : c.properties.keys()) {
                condition.properties.insert(property, c.properties.value(property));
            }
        }

        if (instantaneousCondition.isValid) {
            //Fill in the fields
            for (WeatherCondition::KnownProperties property : instantaneousCondition.properties.keys()) {
                if (property == WeatherCondition::Condition && condition.properties.contains(WeatherCondition::Condition)) continue;
                condition.properties.insert(property, instantaneousCondition.properties.value(property));
            }
        }

        return condition;
    });
}

tPromise<WeatherCondition>* WeatherEngine::getTodayWeather() {
    return new tPromise<WeatherCondition>([=](QString& error) {
        if (!updateWeather()) {
            error = "Weather update issue";
            return WeatherCondition();
        }

        QDate today = QDate::currentDate();

        //Lock the mutex so we can read it
        QMutexLocker locker(&d->knownConditionsMutex);
        WeatherCondition condition;
        condition.isValid = true;

        QList<WeatherCondition> mergeConditions;
        for (WeatherCondition c : d->knownConditions) {
            if (c.startTime.date() == today && c.endTime.date() == today) {
                mergeConditions.append(c);
            }
        }
        locker.unlock();

        for (WeatherCondition c : mergeConditions) {
            //Fill in the fields
            for (WeatherCondition::KnownProperties property : c.properties.keys()) {
                QVariant value = c.properties.value(property);
                if (property == WeatherCondition::HighTemp) {
                    if (!condition.properties.contains(property) || condition.properties.value(property).toDouble() < value.toDouble()) {
                        condition.properties.insert(property, value);
                    }
                } else if (property == WeatherCondition::LowTemp) {
                    if (!condition.properties.contains(property) || condition.properties.value(property).toDouble() > value.toDouble()) {
                        condition.properties.insert(property, value);
                    }
                }
            }
        }

        return condition;
    });
}

WeatherCondition WeatherEngine::instantaneousWeather(QDateTime endTime) {
    //Lock the mutex so we can read it
    QMutexLocker locker(&d->knownConditionsMutex);

    for (WeatherCondition c : d->knownConditions) {
        if (c.startTime == c.endTime && c.endTime == endTime) {
            return c;
        }
    }
    return WeatherCondition();
}

bool WeatherEngine::updateWeather() {
    QMutexLocker updatingLocker(&d->updatingMutex);
    if (d->lastUpdate.secsTo(QDateTime::currentDateTime()) < 3600) return true; //Weather is up to date

    QSharedPointer<QEventLoop> loop(new QEventLoop());


    QNetworkAccessManager mgr;
    QNetworkRequest request(QUrl(QString("https://api.met.no/weatherapi/locationforecastlts/1.3/?lat=%1&lon=%2&msl=%3").arg(d->latitude).arg(d->longitude).arg(d->altitude)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "theShell/8.1");
    QNetworkReply* reply = mgr.get(request);
    connect(reply, &QNetworkReply::finished, loop.data(), &QEventLoop::quit);
    loop->exec();

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStatusCode != 200) {
        reply->deleteLater();
        return false;
    }

    //Parse the XML response
    QDomDocument xmlDoc;
    xmlDoc.setContent(reply->readAll());

    reply->deleteLater();

    //Lock the mutex so we can change it
    QMutexLocker locker(&d->knownConditionsMutex);
    QList<WeatherCondition> instantConditions;
    QDomNodeList times = xmlDoc.elementsByTagName("time");
    for (int i = 0; i < times.count(); i++) {
        QDomNode timeNode = times.at(i);
        QDomElement timeElement = timeNode.toElement();
        if (timeElement.isNull()) continue;
        if (timeElement.attribute("datatype") != "forecast") continue;

        QDomNode location = timeNode.firstChild();
        if (location.toElement().tagName() != "location") continue;

        WeatherCondition condition;
        condition.isValid = true;
        condition.startTime = QDateTime::fromString(timeElement.attribute("from"), Qt::ISODate);
        condition.endTime = QDateTime::fromString(timeElement.attribute("to"), Qt::ISODate);

        QDomNodeList attributes = location.childNodes();
        for (int i = 0; i < attributes.count(); i++) {
            QDomElement attribute = attributes.at(i).toElement();
            if (attribute.isNull()) continue;
            if (attribute.tagName() == "temperature") {
                condition.properties.insert(WeatherCondition::CurrentTemp, attribute.attribute("value").toDouble());
            } else if (attribute.tagName() == "minTemperature") {
                condition.properties.insert(WeatherCondition::LowTemp, attribute.attribute("value").toDouble());
            } else if (attribute.tagName() == "maxTemperature") {
                condition.properties.insert(WeatherCondition::HighTemp, attribute.attribute("value").toDouble());
            } else if (attribute.tagName() == "windSpeed") {
                condition.properties.insert(WeatherCondition::WindSpeed, attribute.attribute("mps").toDouble());
                condition.properties.insert(WeatherCondition::WindBeaufort, attribute.attribute("beaufort").toInt());
            } else if (attribute.tagName() == "symbol") {
                int conditionNumber = attribute.attribute("number").toInt();
                if (conditionNumber > 100) conditionNumber -= 100;

                condition.properties.insert(WeatherCondition::Condition, tr(d->condition.value(conditionNumber)));

                if (QList<int>({3, 4, 10, 11, 13, 14, 15, 22, 23, 30, 31, 32, 33, 34, 46, 48, 50}).contains(conditionNumber)) {
                    condition.properties.insert(WeatherCondition::isCloudy, true);
                } else {
                    condition.properties.insert(WeatherCondition::isCloudy, false);
                }

                if (QList<int>({5, 6, 9, 10, 11, 14, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 41, 42, 43, 46, 47, 48, 49, 50}).contains(conditionNumber)) {
                    condition.properties.insert(WeatherCondition::isRainy, true);
                } else {
                    condition.properties.insert(WeatherCondition::isRainy, false);
                }
            }
        }

        d->knownConditions.append(condition);
    }

    d->lastUpdate = QDateTime::currentDateTime();
    return true;
}
