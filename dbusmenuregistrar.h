#ifndef DBUSMENUREGISTRAR_H
#define DBUSMENUREGISTRAR_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include "registrar_adaptor.h"

#include <X11/Xlib.h>

class DBusMenuRegistrar : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.AppMenu.Registrar")
public:
    explicit DBusMenuRegistrar(QObject *parent = 0);

signals:

public Q_SLOTS:
    void RegisterWindow(uint windowId, QDBusObjectPath menuObjectPath);
    void UnregisterWindow(uint windowId);
    void GetMenuForWindow(uint windowId, QDBusObjectPath &menuObjectPath);
};

#endif // DBUSMENUREGISTRAR_H
