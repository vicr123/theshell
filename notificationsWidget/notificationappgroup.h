#ifndef NOTIFICATIONAPPGROUP_H
#define NOTIFICATIONAPPGROUP_H

#include <QFrame>
#include "notificationpanel.h"
#include "tvariantanimation.h"

namespace Ui {
class NotificationAppGroup;
}

class NotificationAppGroup : public QFrame
{
    Q_OBJECT

public:
    explicit NotificationAppGroup(QString appIdentifier, QIcon appIcon, QString appName, QWidget *parent = 0);
    ~NotificationAppGroup();

    QString getIdentifier();

    int count();

public slots:
    void AddNotification(NotificationObject* object);

private slots:

    void on_closeAllNotificationsButton_clicked();

private:
    Ui::NotificationAppGroup *ui;

    QString appIdentifier;
    QIcon appIcon;

    QList<NotificationPanel*> notifications;
};

#endif // NOTIFICATIONAPPGROUP_H
