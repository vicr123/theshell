#include "notificationdbus.h"
#include "notifications_adaptor.h"

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

        NotificationDialog *d = new NotificationDialog(summary, body, replaces_id, expire_timeout);

        connect(d, SIGNAL(closing(int)), this, SLOT(sendCloseNotification(int)));

        d->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);
        d->show();

        dialogs.append(d);
    } else {
        dialogs.at(replaces_id - 1)->setParams(summary, body);
        dialogs.at(replaces_id - 1)->show();
    }


    return replaces_id;
}

void NotificationDBus::CloseNotification(int id) {
    NotificationDialog *d = dialogs.at(id - 1);
    d->close();
}

void NotificationDBus::sendCloseNotification(int id) {
    emit CloseNotification(id);
}
