/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QPainter>
#include <QPaintEvent>
#include "notificationdbus.h"

class NotificationDBus;

namespace Ui {
class NotificationDialog;
}

class NotificationDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    enum notificationType {
        normalType,
        callType,
    };

    explicit NotificationDialog(QString appName, QString appIconStr, QString title, QString body, QStringList actions, int id, QVariantMap hints, int timeout, notificationType type = normalType, QWidget *parent = 0);
    ~NotificationDialog();

    void show();
    void close(int reason);

    void setParams(QString appName, QString title, QString body);
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    NotificationDBus* dbusParent;

signals:
    void closing(int id, int reason);

private slots:
    void on_pushButton_clicked();

private:
    Ui::NotificationDialog *ui;

    int id;
    int timeout;
    QVariantMap hints;
    bool closed = true;

    void paintEvent(QPaintEvent* event);
};

#endif // NOTIFICATIONDIALOG_H
