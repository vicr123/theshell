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

#include "notificationswidget.h"
#include "ui_notificationswidget.h"

#include <QScroller>
#include <QDBusConnectionInterface>
#include "kjob/jobviewwidget.h"
#include <mpris/mprisengine.h>
#include <quietmodedaemon.h>

NotificationsWidget::NotificationsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationsWidget)
{
    ui->setupUi(this);

    ui->scrollArea->installEventFilter(this);

    this->informationalAttributes.darkColor = QColor(100, 25, 50);
    this->informationalAttributes.lightColor = QColor(200, 50, 100);

    for (MprisPlayerPtr player : MprisEngine::players()) {
        addMediaPlayer(player);
    }
    connect(MprisEngine::instance(), &MprisEngine::newPlayer, this, [=](QString service, MprisPlayerPtr player) {
        Q_UNUSED(service)
        addMediaPlayer(player);
    });

    connect(QuietModeDaemon::instance(), &QuietModeDaemon::QuietModeChanged, [=](QuietModeDaemon::QuietMode newMode) {
        ui->quietModeSound->setChecked(false);
        ui->quietModeCriticalOnly->setChecked(false);
        ui->quietModeNotification->setChecked(false);
        ui->quietModeMute->setChecked(false);
        ui->quietModeForeverButton->setEnabled(true);
        ui->quietModeTurnOffAt->setEnabled(true);
        ui->quietModeTurnOffIn->setEnabled(true);

        if (newMode == QuietModeDaemon::None) {
            ui->quietModeSound->setChecked(true);

            ui->quietModeForeverButton->setEnabled(false);
            ui->quietModeTurnOffAt->setEnabled(false);
            ui->quietModeTurnOffIn->setEnabled(false);
            ui->quietModeForeverButton->setChecked(true);
        } else if (newMode == QuietModeDaemon::Notifications) {
            ui->quietModeNotification->setChecked(true);
        } else if (newMode == QuietModeDaemon::Critical) {
            ui->quietModeCriticalOnly->setChecked(true);
        } else {
            ui->quietModeMute->setChecked(true);
        }
        ui->quietModeDescription->setText(QuietModeDaemon::getCurrentQuietModeDescription());
    });
    ui->quietModeForeverButton->setChecked(true);
    ui->quietModeForeverButton->setEnabled(false);
    ui->quietModeTurnOffAt->setEnabled(false);
    ui->quietModeTurnOffIn->setEnabled(false);
    ui->quietModeExtras->setFixedHeight(0);

    chunk = new QLabel();
    chunk->setText(tr("No Notifications"));
    chunk->installEventFilter(this);

    snack = new QLabel();
    snack->setStyleSheet("QLabel {background-color: #FF6400; color: white;}");
    snack->setMargin(6);
    snack->setVisible(false);
    snack->setText("0");

    QTimer::singleShot(0, [=] {
        ui->quietModeDescription->setText(QuietModeDaemon::getCurrentQuietModeDescription());

        sendMessage("register-chunk", {QVariant::fromValue(chunk)});
        sendMessage("register-snack", {QVariant::fromValue(snack)});
    });

    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

NotificationsWidget::~NotificationsWidget()
{
    delete ui;
}

QWidget* NotificationsWidget::mainWidget() {
    return this;
}

QString NotificationsWidget::name() {
    return tr("Notifications");
}

StatusCenterPaneObject::StatusPaneTypes NotificationsWidget::type() {
    return Informational;
}

int NotificationsWidget::position() {
    return 0;
}

void NotificationsWidget::message(QString name, QVariantList args) {

}

void NotificationsWidget::addMediaPlayer(MprisPlayerPtr player) {
    MediaPlayerNotification* n = new MediaPlayerNotification(player);
    mediaPlayers.insert(player, n);
    ((QBoxLayout*) ui->notificationGroups->layout())->insertWidget(0, n);
    ui->noNotificationsFrame->setVisible(false);
    connect(n, &MediaPlayerNotification::destroyed, this, [=] {
        mediaPlayers.remove(player);
        ui->notificationGroups->layout()->removeWidget(n);

        if (notifications.count() == 0 && mediaPlayers.count() == 0) {
            ui->noNotificationsFrame->setVisible(true);
        }
    });
}

void NotificationsWidget::setAdaptor(NotificationsDBusAdaptor* adaptor) {
    this->adaptor = adaptor;
}

void NotificationsWidget::addNotification(NotificationObject *object) {
    notifications.insert(object->getId(), object);
    ui->noNotificationsFrame->setVisible(false);

    connect(object, &NotificationObject::actionClicked, object, [=](QString key) {
        emit adaptor->ActionInvoked(object->getId(), key);
        object->dismiss();
    });
    connect(object, &NotificationObject::closed, object, [=](NotificationObject::NotificationCloseReason reason) {
        emit adaptor->NotificationClosed(object->getId(), reason);
        notifications.remove(object->getId());

        if (notifications.count() == 0 && mediaPlayers.count() == 0) {
            ui->noNotificationsFrame->setVisible(true);
        }

        updateNotificationsNumber();
    });

    NotificationAppGroup* nGroup = nullptr;

    for (NotificationAppGroup* group : notificationGroups) {
        if (group->getIdentifier() == object->getAppIdentifier()) {
            nGroup = group;
            break;
        }
    }

    if (nGroup == nullptr) {
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
    updateNotificationsNumber();
}

void NotificationsWidget::updateNotificationsNumber() {
    if (notifications.count() == 0) {
        chunk->setText(tr("No notifications"));
        snack->setVisible(false);
    } else {
        chunk->setText("<b>" + tr("%n notification(s)", nullptr, notifications.count()) + "</b>");
        snack->setText(QString::number(notifications.count()));
        snack->setVisible(true);
    }
}

bool NotificationsWidget::hasNotificationId(uint id) {
    return notifications.contains(id);
}

NotificationObject* NotificationsWidget::getNotification(uint id) {
    return notifications.value(id);
}

void NotificationsWidget::addJobView(JobViewWidget *view) {
    ((QBoxLayout*) ui->notificationGroups->layout())->insertWidget(0, view);
}

bool NotificationsWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->scrollArea) {
        if (event->type() == QEvent::Resize) {
            ui->notificationGroups->setFixedWidth(ui->scrollArea->width());
        }
    } else if (watched == chunk) {
        if (event->type() == QEvent::MouseButtonPress) {
            sendMessage("show", {});
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

void NotificationsWidget::on_quietModeSound_clicked()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::None);
    ui->quietModeSound->setChecked(true);
}

void NotificationsWidget::on_quietModeNotification_clicked()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Notifications);
    ui->quietModeNotification->setChecked(true);
}

void NotificationsWidget::on_quietModeMute_clicked()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Mute);
    ui->quietModeMute->setChecked(true);
}

void NotificationsWidget::on_quietModeCriticalOnly_clicked()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Critical);
    ui->quietModeCriticalOnly->setChecked(true);
}

void NotificationsWidget::on_quietModeExpandButton_clicked()
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->quietModeExtras->height());
    if (ui->quietModeExtras->height() == 0) {
        anim->setEndValue(ui->quietModeExtras->sizeHint().height());
        ui->quietModeExpandButton->setIcon(QIcon::fromTheme("go-up"));
    } else {
        anim->setEndValue(0);
        ui->quietModeExpandButton->setIcon(QIcon::fromTheme("go-down"));
    }
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->quietModeExtras->setFixedHeight(value.toInt());
    });
    anim->start();
}

void NotificationsWidget::on_quietModeForeverButton_toggled(bool checked)
{
    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(false);
        ui->quietModeTurnOffInTimer->setVisible(false);
        QuietModeDaemon::setQuietModeResetTime(QDateTime::fromString(""));
    }
}

void NotificationsWidget::on_quietModeTurnOffIn_toggled(bool checked)
{
    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(false);
        ui->quietModeTurnOffInTimer->setVisible(true);

        QDateTime oneHour = QDateTime::currentDateTime().addSecs(3600);
        QuietModeDaemon::setQuietModeResetTime(oneHour);
        ui->quietModeTurnOffInTimer->setTime(QTime(1, 0));
    }
}

void NotificationsWidget::on_quietModeTurnOffAt_toggled(bool checked)
{
    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(true);
        ui->quietModeTurnOffInTimer->setVisible(false);

        QDateTime oneHour = QDateTime::currentDateTime().addSecs(3600);
        QuietModeDaemon::setQuietModeResetTime(oneHour);
        ui->quietModeTurnOffAtTimer->setDateTime(oneHour);
    }
}

void NotificationsWidget::on_quietModeTurnOffAtTimer_editingFinished()
{
    QuietModeDaemon::setQuietModeResetTime(ui->quietModeTurnOffAtTimer->dateTime());
}

void NotificationsWidget::on_quietModeTurnOffInTimer_editingFinished()
{
    QDateTime timeout = QDateTime::currentDateTime().addMSecs(ui->quietModeTurnOffInTimer->time().msecsSinceStartOfDay());
    QuietModeDaemon::setQuietModeResetTime(timeout);
}
