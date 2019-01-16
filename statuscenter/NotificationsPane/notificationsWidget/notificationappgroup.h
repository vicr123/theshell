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
