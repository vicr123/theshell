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

#ifndef NOTIFICATIONPANEL_H
#define NOTIFICATIONPANEL_H

#include <QWidget>
#include <QMouseEvent>
#include "notificationobject.h"
#include "tpropertyanimation.h"

namespace Ui {
class NotificationPanel;
}

class NotificationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationPanel(NotificationObject* object, QWidget *parent = 0);
    ~NotificationPanel();

    NotificationObject* getObject();

public slots:
    void collapseHide();
    void expandHide();
    void toggleExpandNormal();

private slots:
    void updateParameters();

    void closeNotification(NotificationObject::NotificationCloseReason reason);

    void on_closeButton_clicked();

signals:
    void dismissed();

private:
    Ui::NotificationPanel *ui;

    bool expanded = false;

    void mouseReleaseEvent(QMouseEvent* event);

    NotificationObject* object;
};

#endif // NOTIFICATIONPANEL_H
