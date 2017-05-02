#include "locationservices.h"
//#include "agent_adaptor.h"

LocationServices::LocationServices(QObject *parent) : QObject(parent)
{
    /*QDBusMessage activator = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "StartServiceByName");
    activator.setArguments(QVariantList() << "org.freedesktop.GeoClue2");
    QDBusConnection::systemBus().call(activator);

    new AgentAdaptor(this);
    QDBusConnection dbus = QDBusConnection::systemBus();
    qDebug() << dbus.registerObject("/org/freedesktop/GeoClue2/Agent", this);

    QDBusMessage agentMessage = QDBusMessage::createMethodCall("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.GeoClue2.Manager", "AddAgent");
    agentMessage.setArguments(QVariantList() << "gnome-shell");
    QDBusPendingCall message = QDBusConnection::systemBus().asyncCall(agentMessage);*/
    //qDebug() << message.errorMessage();

    QDBusConnection::systemBus().connect("org.freedesktop.GeoClue2", "/org/freedesktop/GeoClue2/Manager", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(GeocluePropertiesChanged(QString,QVariantMap)));
}

bool LocationServices::AuthorizeApp(QString desktop_id, uint req_accuracy_level, uint &allowed_accuracy_level) {
    qDebug() << desktop_id + " is accessing location";
    allowed_accuracy_level = req_accuracy_level;
    return true;
}

uint LocationServices::MaxAccuracyLevel() {
    return 6;
}

void LocationServices::GeocluePropertiesChanged(QString interface, QVariantMap properties) {
    if (interface == "org.freedesktop.GeoClue2.Manager") {
        if (properties.keys().contains("InUse")) {
            emit locationUsingChanged(properties.value("InUse").toBool());
        }
    }
}
