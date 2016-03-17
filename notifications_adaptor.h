#ifndef NOTIFICATIONS_ADAPTOR_H
#define NOTIFICATIONS_ADAPTOR_H
#include <QObject>
#include "notificationdbus.h"

class NotificationsAdaptor : public QObject {
    Q_OBJECT

public:
    NotificationsAdaptor(QObject *parent = 0);
};

#endif // NOTIFICATIONS_ADAPTOR_H
