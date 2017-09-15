#include "notificationsdbusadaptor.h"
#include "notificationswidget.h"
#include "notificationobject.h"

NotificationsDBusAdaptor::NotificationsDBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    this->setAutoRelaySignals(true);
}

NotificationsDBusAdaptor::~NotificationsDBusAdaptor()
{

}

void NotificationsDBusAdaptor::CloseNotification(uint id)
{
    if (this->parentWidget()->hasNotificationId(id)) {
        NotificationObject* notification = this->parentWidget()->getNotification(id);
        notification->dismiss();
    }
}

QStringList NotificationsDBusAdaptor::GetCapabilities()
{
    return QStringList() << "body" << "body-hyperlinks" << "body-markup";
}

QString NotificationsDBusAdaptor::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    vendor = "theSuite";
    version = "2.0";
    spec_version = "1.2";
    return "theShell";
}

uint NotificationsDBusAdaptor::Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout)
{
    if (this->parentWidget() != NULL) {
        NotificationObject* notification;
        if (this->parentWidget()->hasNotificationId(replaces_id)) {
            notification = this->parentWidget()->getNotification(replaces_id);
        } else {
            notification = new NotificationObject(app_name, app_icon, summary, body, actions, hints, expire_timeout);
        }
        this->parentWidget()->addNotification(notification);

        notification->post();

        return notification->getId();
    }
    return 0;
}

NotificationsWidget* NotificationsDBusAdaptor::parentWidget() {
    return pt;
}

void NotificationsDBusAdaptor::setParentWidget(NotificationsWidget *parent) {
    pt = parent;
}
