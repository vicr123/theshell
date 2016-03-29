#ifndef NOTIFICATIONS_ADAPTOR_H
#define NOTIFICATIONS_ADAPTOR_H
#include <QObject>
#include "notificationdbus.h"

class NotificationDBus;

class NotificationsAdaptor : public QObject {
    Q_OBJECT

public:
    NotificationsAdaptor(NotificationDBus *parent = 0);
    //NotificationsAdaptor(NotificationsAdaptor *parent = 0);
};

#endif // NOTIFICATIONS_ADAPTOR_H
