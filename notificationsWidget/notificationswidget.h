#ifndef NOTIFICATIONSWIDGET_H
#define NOTIFICATIONSWIDGET_H

#include <QWidget>
#include "notificationsdbusadaptor.h"
#include "notificationpopup.h"
#include "notificationobject.h"
#include "notificationappgroup.h"

namespace Ui {
class NotificationsWidget;
}

class NotificationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationsWidget(QWidget *parent = 0);
    ~NotificationsWidget();

    void addNotification(NotificationObject* object);
    bool hasNotificationId(uint id);
    NotificationObject* getNotification(uint id);

private:
    Ui::NotificationsWidget *ui;

    QMap<int, NotificationObject*> notifications;
    QList<NotificationAppGroup*> notificationGroups;
};

#endif // NOTIFICATIONSWIDGET_H