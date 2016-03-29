#ifndef NOTIFICATIONDBUS_H
#define NOTIFICATIONDBUS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QApplication>
#include "notificationdialog.h"
#include "notifications_adaptor.h"

class NotificationDialog;

class NotificationDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
public:
    explicit NotificationDBus(QObject *parent = 0);
    void invokeAction(uint id, QString key);

Q_SIGNALS:
    Q_SCRIPTABLE void NotificationClosed(int id, int reason);
    Q_SCRIPTABLE void ActionInvoked(uint id, QString action_key);

public Q_SLOTS:
    QStringList GetCapabilities();
    uint Notify(QString app_name, uint replaces_id,
               QString app_icon, QString summary,
               QString body, QStringList actions,
               QVariantMap hints, int expire_timeout);
    void CloseNotification(int id);
    QString GetServerInformation(QString &vendor,
                              QString &version, QString &spec_version);
    //void GetServerInformation(QDBusMessage m);

protected Q_SLOTS:
    void sendCloseNotification(int id, int reason);

private:
    QList<NotificationDialog*> dialogs;
    int nextId = 1;
};

#endif // NOTIFICATIONDBUS_H
