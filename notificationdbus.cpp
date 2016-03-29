#include "notificationdbus.h"

NotificationDBus::NotificationDBus(QObject *parent) : QObject(parent)
{
    new NotificationsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/freedesktop/Notifications", this);
    dbus.registerService("org.freedesktop.Notifications");
}

QStringList NotificationDBus::GetCapabilities() {
    return QStringList() << "body" << "body-markup" << "icon-static" << "actions";
}

uint NotificationDBus::Notify(QString app_name, uint replaces_id,
                              QString app_icon, QString summary,
                              QString body, QStringList actions,
                              QVariantMap hints, int expire_timeout) {
    if (replaces_id == 0) {
        replaces_id = nextId;
        nextId++;

        NotificationDialog *d = new NotificationDialog(summary, body, actions, replaces_id, expire_timeout);
        d->dbusParent = this;

        connect(d, SIGNAL(closing(int, int)), this, SLOT(sendCloseNotification(int, int)));

        d->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        d->show();

        dialogs.append(d);
    } else {
        dialogs.at(replaces_id - 1)->setParams(summary, body);
        dialogs.at(replaces_id - 1)->show();
    }


    return replaces_id;
}

QString NotificationDBus::GetServerInformation(QString &vendor, QString &version, QString &spec_version) {

    vendor = "theSuite";
    version = "1.0";
    spec_version = "1.2";
    return "theShell";
}

void NotificationDBus::CloseNotification(int id) {
    NotificationDialog *d = dialogs.at(id - 1);
    d->close(3);
}

void NotificationDBus::sendCloseNotification(int id, int reason) {
    emit NotificationClosed(id, reason);
}

void NotificationDBus::invokeAction(uint id, QString key) {
    emit ActionInvoked(id, key);
}
