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

#ifndef NOTIFICATIONOBJECT_H
#define NOTIFICATIONOBJECT_H

#include <QObject>
#include <QMediaPlayer>
#include <QSoundEffect>
#include "notificationpopup.h"
#include "audiomanager.h"

class NotificationPopup;

class NotificationObject : public QObject
{
    Q_OBJECT
public:
    enum NotificationCloseReason : uint {
        Expired = 1,
        Dismissed = 2,
        ProgrammaticallyClosed = 3,
        Undefined = 4
    };

    explicit NotificationObject(QString app_name, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout, QObject *parent = nullptr);
    static int currentId;

    uint getId();

    QString getAppIdentifier();
    QString getAppName();
    QIcon getAppIcon();
    QString getSummary();
    QString getBody();

signals:
    void parametersUpdated();
    void actionClicked(QString key);
    void closed(NotificationObject::NotificationCloseReason reason);

public slots:
    void post();
    void setParameters(QString &app_name, QString &app_icon, QString &summary, QString &body, QStringList &actions, QVariantMap &hints, int expire_timeout);
    void closeDialog();
    void dismiss();

private:
    QString appName, appIcon, summary, body;
    QStringList actions;
    QVariantMap hints;
    int timeout;
    uint id;

    NotificationPopup* dialog;

    QIcon appIc, bigIc;
    QSettings settings;
    QSettings* notificationAppSettings = new QSettings("theSuite", "theShell-notifications", this);
};

#endif // NOTIFICATIONOBJECT_H
