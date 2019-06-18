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

#include "locationdaemon.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

struct LocationDaemonPrivate {
    LocationDaemon* instance = nullptr;

    QDBusObjectPath geoclueClientPath;
    QDBusInterface* clientInterface = nullptr;
    int listeningTimes = 0;

    QMutex listeningMutex;
};

LocationDaemonPrivate* LocationDaemon::d = new LocationDaemonPrivate();

LocationDaemon::LocationDaemon(QObject *parent) : QObject(parent)
{

}

tPromise<Geolocation>* LocationDaemon::singleShot() {
    return new tPromise<Geolocation>([=](QString& error) -> Geolocation {
        bool updateRequired = true;
        QDBusObjectPath locationPath;
        if (d->clientInterface != nullptr) {
            locationPath = d->clientInterface->property("Location").value<QDBusObjectPath>();
            if (locationPath.path() != "/") updateRequired = false;
        }

        if (updateRequired) {
            if (startListening()) {
                QEventLoop loop;
                QDBusConnection::systemBus().connect(d->clientInterface->service(), d->geoclueClientPath.path(), d->clientInterface->interface(), "LocationUpdated", &loop, SLOT(quit()));
                loop.exec();

                locationPath = d->clientInterface->property("Location").value<QDBusObjectPath>();
            } else {
                error = tr("Couldn't start Geoclue");
                return Geolocation();
            }
        }

        //Location has been updated
        Geolocation loc;

        QDBusInterface locationInterface("org.freedesktop.GeoClue2", locationPath.path(), "org.freedesktop.GeoClue2.Location", QDBusConnection::systemBus());
        loc.latitude = locationInterface.property("Latitude").toDouble();
        loc.longitude = locationInterface.property("Longitude").toDouble();
        loc.accuracy = locationInterface.property("Accuracy").toDouble();
        loc.altitude = locationInterface.property("Altitude").toDouble();
        loc.speed = locationInterface.property("Speed").toDouble();
        loc.heading = locationInterface.property("Heading").toDouble();
        loc.description = locationInterface.property("Description").toString();

        loc.resolved = true;
        stopListening();
        return loc;
    });
}

bool LocationDaemon::startListening() {
    QMutexLocker locker(&d->listeningMutex);
    makeInstance();

    if (d->listeningTimes == 0) {
        d->listeningTimes++;
        QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
        if (!reply.value().contains("org.freedesktop.GeoClue2")) {
            qDebug() << "Can't start GeoClue";
            d->listeningTimes--;
            return false;
        }

        QDBusConnection::systemBus().interface()->startService("org.freedesktop.GeoClue2");

        QDBusMessage clientMessage = QDBusMessage::createMethodCall("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.GeoClue2.Manager", "GetClient");
        QDBusReply<QDBusObjectPath> clientPathReply = QDBusConnection::systemBus().call(clientMessage);
        d->geoclueClientPath = clientPathReply.value();

        if (d->clientInterface != nullptr) d->clientInterface->deleteLater();

        d->clientInterface = new QDBusInterface("org.freedesktop.GeoClue2", d->geoclueClientPath.path(), "org.freedesktop.GeoClue2.Client", QDBusConnection::systemBus());
        d->clientInterface->setProperty("DesktopId", "theshell");
        QDBusConnection::systemBus().connect(d->clientInterface->service(), d->geoclueClientPath.path(), d->clientInterface->interface(), "LocationUpdated", d->instance, SLOT(locationUpdated()));

        QDBusMessage startReply = d->clientInterface->call("Start");
        if (startReply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Couldn't start listening for location updates:" << startReply.errorName() << startReply.errorMessage();
            d->listeningTimes--;
            return false;
        }
    }
    return true;
}

void LocationDaemon::locationUpdated() {

}

bool LocationDaemon::stopListening() {
    if (d->listeningTimes == 1) {
        d->clientInterface->call("Stop");
        d->listeningTimes--;
        return true;
    } else if (d->listeningTimes > 1) {
        d->listeningTimes--;
        return true;
    } else {
        return false;
    }
}

void LocationDaemon::makeInstance() {
    if (d->instance == nullptr) d->instance = new LocationDaemon();
}

tPromise<GeoPlace>* LocationDaemon::reverseGeocode(double latitude, double longitude) {
    return new tPromise<GeoPlace>([=](QString& error) {
        QSharedPointer<QEventLoop> loop(new QEventLoop());

        QNetworkAccessManager mgr;
        QNetworkRequest request(QUrl(QString("http://api.geonames.org/findNearbyPlaceNameJSON?lat=%1&lng=%2&lang=local&username=vicr123").arg(latitude).arg(longitude)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "theShell/8.1");
        QNetworkReply* reply = mgr.get(request);
        connect(reply, &QNetworkReply::finished, loop.data(), &QEventLoop::quit);
        loop->exec();

        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpStatusCode != 200) {
            reply->deleteLater();
            error = "Geocoding Failed";
            return GeoPlace();
        }

        //Parse the XML response
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();

        QJsonObject rootObj = jsonDoc.object();
        if (!rootObj.contains("geonames")) {
            error = "Geocoding Failed";
            return GeoPlace();
        }
        QJsonArray geonamesArray = rootObj.value("geonames").toArray();
        if (geonamesArray.count() == 0) {
            error = "Geocoding Failed";
            return GeoPlace();
        }

        QJsonObject placeObject = geonamesArray.first().toObject();
        GeoPlace place;
        place.name = placeObject.value("name").toString();
        place.administrativeName = placeObject.value("adminName1").toString();
        return place;
    });
}
