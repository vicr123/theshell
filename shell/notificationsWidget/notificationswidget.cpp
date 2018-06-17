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

#include "notificationswidget.h"
#include "ui_notificationswidget.h"

extern NotificationsDBusAdaptor* ndbus;
extern NativeEventFilter* NativeFilter;

NotificationsWidget::NotificationsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationsWidget)
{
    ui->setupUi(this);

    ndbus->setParentWidget(this);

    ui->scrollArea->installEventFilter(this);

    QTimer* t = new QTimer();
    t->setInterval(1000);
    connect(t, &QTimer::timeout, [=] {
        QStringList knownServices;
        for (QString service : QDBusConnection::sessionBus().interface()->registeredServiceNames().value()) {
            if (service.startsWith("org.mpris.MediaPlayer2")) {
                knownServices.append(service);
                if (!mediaPlayers.keys().contains(service)) {
                    MediaPlayerNotification* n = new MediaPlayerNotification(service);
                    mediaPlayers.insert(service, n);
                    ((QBoxLayout*) ui->notificationGroups->layout())->insertWidget(0, n);
                    ui->noNotificationsFrame->setVisible(false);
                    connect(n, &MediaPlayerNotification::destroyed, [=] {
                        mediaPlayers.remove(service);
                        ui->notificationGroups->layout()->removeWidget(n);

                        if (notifications.count() == 0 && mediaPlayers.count() == 0) {
                            ui->noNotificationsFrame->setVisible(true);
                        }
                    });
                }
            }
        }

        for (QString service : mediaPlayers.keys()) {
            if (!knownServices.contains(service)) {
                mediaPlayers.value(service)->deleteLater();
            }
        }

    });
    t->start();
}

NotificationsWidget::~NotificationsWidget()
{
    delete ui;
}

void NotificationsWidget::addNotification(NotificationObject *object) {
    notifications.insert(object->getId(), object);
    ui->noNotificationsFrame->setVisible(false);

    connect(object, &NotificationObject::actionClicked, [=](QString key) {
        emit ndbus->ActionInvoked(object->getId(), key);
        object->dismiss();
    });
    connect(object, &NotificationObject::closed, [=](NotificationObject::NotificationCloseReason reason) {
        emit ndbus->NotificationClosed(object->getId(), reason);
        notifications.remove(object->getId());

        if (notifications.count() == 0 && mediaPlayers.count() == 0) {
            ui->noNotificationsFrame->setVisible(true);
        }
    });

    NotificationAppGroup* nGroup = NULL;

    for (NotificationAppGroup* group : notificationGroups) {
        if (group->getIdentifier() == object->getAppIdentifier()) {
            nGroup = group;
            break;
        }
    }

    if (nGroup == NULL) {
        nGroup = new NotificationAppGroup(object->getAppIdentifier(), object->getAppIcon(), object->getAppName());
        ((QBoxLayout*) ui->notificationGroups->layout())->insertWidget(mediaPlayers.count(), nGroup);

        connect(nGroup, SIGNAL(notificationCountChanged()), this, SLOT(updateNotificationCount()));
        connect(nGroup, &NotificationAppGroup::destroyed, [=] {
            ui->notificationGroups->layout()->removeWidget(nGroup);
            notificationGroups.removeOne(nGroup);
        });

        notificationGroups.append(nGroup);
    }

    nGroup->AddNotification(object);
}

bool NotificationsWidget::hasNotificationId(uint id) {
    return notifications.contains(id);
}

NotificationObject* NotificationsWidget::getNotification(uint id) {
    return notifications.value(id);
}

bool NotificationsWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->scrollArea) {
        if (event->type() == QEvent::Resize) {
            ui->notificationGroups->setFixedWidth(ui->scrollArea->width());
        }
    }
    return false;
}

void NotificationsWidget::on_clearAllButton_clicked()
{
    for (NotificationAppGroup* group : notificationGroups) {
        group->clearAll();
    }
}

void NotificationsWidget::updateNotificationCount() {
    int count = 0;

    for (NotificationAppGroup* group : notificationGroups) {
        count += group->count();
    }

    emit numNotificationsChanged(count);
}


void NotificationsWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
