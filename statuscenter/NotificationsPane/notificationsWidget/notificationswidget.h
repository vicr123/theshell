/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
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
#include "mediaplayernotification.h"
#include <statuscenterpaneobject.h>

class NotificationsDBusAdaptor;
class JobViewWidget;

namespace Ui {
class NotificationsWidget;
}

class NotificationsWidget : public QWidget, public StatusCenterPaneObject
{
    Q_OBJECT

public:
    explicit NotificationsWidget(QWidget *parent = 0);
    ~NotificationsWidget();

    void setAdaptor(NotificationsDBusAdaptor* adaptor);

    void addNotification(NotificationObject* object);
    bool hasNotificationId(uint id);
    NotificationObject* getNotification(uint id);

    void addJobView(JobViewWidget* view);

    QWidget* mainWidget();
    QString name();
    StatusPaneTypes type();
    int position();
    void message(QString name, QVariantList args = QVariantList());

private slots:
    void on_clearAllButton_clicked();

    void updateNotificationCount();

    void on_quietModeSound_clicked();

    void on_quietModeNotification_clicked();

    void on_quietModeMute_clicked();

    void on_quietModeCriticalOnly_clicked();

    void on_quietModeExpandButton_clicked();

    void on_quietModeForeverButton_toggled(bool checked);

    void on_quietModeTurnOffIn_toggled(bool checked);

    void on_quietModeTurnOffAt_toggled(bool checked);

    void on_quietModeTurnOffAtTimer_editingFinished();

    void on_quietModeTurnOffInTimer_editingFinished();

signals:
    void numNotificationsChanged(int number);

private:
    Ui::NotificationsWidget *ui;

    bool eventFilter(QObject *watched, QEvent *event);
    void changeEvent(QEvent* event);

    QMap<int, NotificationObject*> notifications;
    QList<NotificationAppGroup*> notificationGroups;
    QMap<QString, MediaPlayerNotification*> mediaPlayers;

    QLabel* chunk;

    NotificationsDBusAdaptor* adaptor;
};

#endif // NOTIFICATIONSWIDGET_H
