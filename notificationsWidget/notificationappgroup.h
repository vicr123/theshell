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

    void clearAll();

    void updateCollapsedCounter();

private slots:

    void on_closeAllNotificationsButton_clicked();

    void on_collapsedLabel_clicked();

    void on_expandNotificationsButton_clicked();

signals:
    void notificationCountChanged();

private:
    Ui::NotificationAppGroup *ui;

    QString appIdentifier;
    QIcon appIcon;

    bool expanded = false;

    QList<NotificationPanel*> notifications;
};

#endif // NOTIFICATIONAPPGROUP_H
