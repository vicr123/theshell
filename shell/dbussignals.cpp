#include "dbussignals.h"
#include "theshell_adaptor.h"

DBusSignals::DBusSignals(QObject *parent) : QObject(parent)
{
    new TheshellAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/thesuite/theshell", this);
    dbus.registerService("org.thesuite.theshell");
}
