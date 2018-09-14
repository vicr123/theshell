#include "locationdaemon.h"

LocationDaemon::LocationDaemon(QObject *parent) : QObject(parent)
{

}

tPromise<Geolocation>* LocationDaemon::singleShot() {
    return new tPromise<Geolocation>([=](QString& error) -> Geolocation {
        QEventLoop loop;

        if (this->startListening()) {
            QDBusConnection::systemBus().connect(clientInterface->service(), geoclueClientPath.path(), clientInterface->interface(), "LocationUpdated", &loop, SLOT(quit()));
            loop.exec();

            //Location has been updated
            Geolocation loc;
            QDBusObjectPath locationPath = clientInterface->property("Location").value<QDBusObjectPath>();

            QDBusInterface locationInterface("org.freedesktop.GeoClue2", locationPath.path(), "org.freedesktop.GeoClue2.Location", QDBusConnection::systemBus());
            loc.latitude = locationInterface.property("Latitude").toDouble();
            loc.longitude = locationInterface.property("Longitude").toDouble();
            loc.accuracy = locationInterface.property("Accuracy").toDouble();
            loc.altitude = locationInterface.property("Altitude").toDouble();
            loc.speed = locationInterface.property("Speed").toDouble();
            loc.heading = locationInterface.property("Heading").toDouble();
            loc.description = locationInterface.property("Description").toString();

            loc.resolved = true;
            this->stopListening();
            return loc;
        } else {
            error = tr("Couldn't start Geoclue");
            return Geolocation();
        }
    });
}

bool LocationDaemon::startListening() {
    QMutexLocker locker(&listeningMutex);
    if (listeningTimes == 0) {
        listeningTimes++;
        QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
        if (!reply.value().contains("org.freedesktop.GeoClue2")) {
            qDebug() << "Can't start GeoClue";
            listeningTimes--;
            return false;
        }

        QDBusConnection::systemBus().interface()->startService("org.freedesktop.GeoClue2");

        QDBusMessage clientMessage = QDBusMessage::createMethodCall("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.GeoClue2.Manager", "GetClient");
        QDBusReply<QDBusObjectPath> clientPathReply = QDBusConnection::systemBus().call(clientMessage);
        geoclueClientPath = clientPathReply.value();

        if (clientInterface != nullptr) clientInterface->deleteLater();

        clientInterface = new QDBusInterface("org.freedesktop.GeoClue2", geoclueClientPath.path(), "org.freedesktop.GeoClue2.Client", QDBusConnection::systemBus());
        clientInterface->setProperty("DesktopId", "theshell");
        QDBusConnection::systemBus().connect(clientInterface->service(), geoclueClientPath.path(), clientInterface->interface(), "LocationUpdated", this, SLOT(locationUpdated()));

        QDBusMessage startReply = clientInterface->call("Start");
        if (startReply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Couldn't start listening for location updates:" << startReply.errorName() << startReply.errorMessage();
            listeningTimes--;
            return false;
        }
    }
    return true;
}

void LocationDaemon::locationUpdated() {

}

bool LocationDaemon::stopListening() {
    if (listeningTimes == 1) {
        clientInterface->call("Stop");
        listeningTimes--;
        return true;
    } else if (listeningTimes > 1) {
        listeningTimes--;
        return true;
    } else {
        return false;
    }
}
