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

#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#include <QDialog>
#include "tpropertyanimation.h"
#include "notificationsdbusadaptor.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QDirIterator>
#include <QtMath>

namespace Ui {
class NotificationPopup;
}

class NotificationPopup : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationPopup(int id, QWidget *parent = nullptr);
    ~NotificationPopup();

    void show();
    void close();

    void setApp(QString appName, QIcon appIcon);
    void setSummary(QString summary);
    void setBody(QString body);
    void setActions(QStringList actions, bool actionNamesAreIcons);
    void setTimeout(int timeout);
    void setHints(QVariantMap hints);
    void setBigIcon(QIcon bigIcon);

private slots:
    void on_dismissButton_clicked();
    void startDismisser();
    void stopDismisser();

signals:
    void actionClicked(QString key);
    void notificationClosed(uint reason);

private:
    Ui::NotificationPopup *ui;

    int id;
    int textHeight;
    bool mouseEvents = false;

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* resizeEvent);
    bool event(QEvent* event);

    QTimer* dismisser = nullptr;
    int timeoutLeft;
    QMap<QString, QString> actions;
    QVariantMap hints;
    int urgency = 0;

    static NotificationPopup* currentlyShowingPopup;
    static QList<NotificationPopup*> pendingPopups;

    QWidget* coverWidget;
    QLabel *coverAppIcon, *coverAppName;
    QSettings settings;

    int currentTouch = -1;
    int dismisserStopCount = 0;

    QPoint touchStart;
};

#endif // NOTIFICATIONPOPUP_H
