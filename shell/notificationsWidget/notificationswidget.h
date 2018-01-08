/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
    void changeEvent(QEvent* event);

    QMap<int, NotificationObject*> notifications;
    QList<NotificationAppGroup*> notificationGroups;
};

#endif // NOTIFICATIONSWIDGET_H
