#include "dbusmenuregistrar.h"

DBusMenuRegistrar::DBusMenuRegistrar(QObject *parent) : QObject(parent)
{
    new RegistrarAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/com/canonical/AppMenu/Registrar", this);
    dbus.registerService("com.canonical.AppMenu.Registrar");
}

void DBusMenuRegistrar::RegisterWindow(uint windowId, QDBusObjectPath menuObjectPath) {

}

void DBusMenuRegistrar::UnregisterWindow(uint windowId) {

}

void DBusMenuRegistrar::GetMenuForWindow(uint windowId, QDBusObjectPath &menuObjectPath) {

}
