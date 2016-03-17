#ifndef NOTIFICATIONDBUS_H
#define NOTIFICATIONDBUS_H

#include <QObject>
#include <QDBusConnection>
#include <QApplication>
#include "notificationdialog.h"

class NotificationDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
public:
    explicit NotificationDBus(QObject *parent = 0);

Q_SIGNALS:
    Q_SCRIPTABLE void NotificationClosed(int id, int reason);
    Q_SCRIPTABLE void ActionInvoked(int id, QString action_key);

public Q_SLOTS:
    QStringList GetCapabilities();
    uint Notify(QString app_name, uint replaces_id,
               QString app_icon, QString summary,
               QString body, QStringList actions,
               QVariantMap hints, int expire_timeout);
    void CloseNotification(int id);

protected Q_SLOTS:
    void sendCloseNotification(int id);

private:
    QList<NotificationDialog*> dialogs;
    int nextId = 1;
};

#endif // NOTIFICATIONDBUS_H
