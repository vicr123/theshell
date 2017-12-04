#ifndef NOTIFICATIONSWIDGET_H
#define NOTIFICATIONSWIDGET_H

#include <QWidget>
#include "notificationsdbusadaptor.h"
#include "notificationpopup.h"
#include "notificationobject.h"
#include "notificationappgroup.h"
#include <QScrollArea>
#include "nativeeventfilter.h"

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

private slots:
    void on_clearAllButton_clicked();

    void updateNotificationCount();

signals:
    void numNotificationsChanged(int number);

private:
    Ui::NotificationsWidget *ui;

    bool eventFilter(QObject *watched, QEvent *event);

    QMap<int, NotificationObject*> notifications;
    QList<NotificationAppGroup*> notificationGroups;
};

#endif // NOTIFICATIONSWIDGET_H
