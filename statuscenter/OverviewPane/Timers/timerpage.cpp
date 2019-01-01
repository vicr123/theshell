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

#include "timerpage.h"
#include "ui_timerpage.h"

#include <QScroller>
#include <QDBusInterface>
#include <QMediaPlaylist>

TimerPage::TimerPage(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::TimerPage)
{
    ui->setupUi(this);

    this->setCurrentWidget(ui->noTimersPage);
    QScroller::grabGesture(ui->timersScroll->viewport(), QScroller::LeftMouseButtonGesture);

    notificationInterface = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
    QDBusConnection::sessionBus().connect(notificationInterface->service(), notificationInterface->path(), notificationInterface->interface(), "NotificationClosed", this, SLOT(notificationClosed(uint,uint)));

    ringtone = new QMediaPlayer(this, QMediaPlayer::LowLatency);
}

TimerPage::~TimerPage()
{
    delete ui;
}

void TimerPage::on_backButton_clicked()
{
    if (timersCreated == 0) {
        this->setCurrentWidget(ui->noTimersPage);
    } else {
        this->setCurrentWidget(ui->timersList);
    }
}

void TimerPage::on_newTimerButton_clicked()
{
    this->setCurrentWidget(ui->newTimerPage);

    ui->newTimerBox->setTime(QTime::fromMSecsSinceStartOfDay(0));

    ui->newTimerName->setText(tr("Timer %n", nullptr, timersCreated + 1));
}

void TimerPage::on_setTimerButton_clicked()
{
    QString ringtone;
    switch (ui->ringtoneBox->currentIndex()) {
        case 0: ringtone = "Happy Bee"; break;
        case 1: ringtone = "Playing in the Dark"; break;
        case 2: ringtone = "Ice Cream Truck"; break;
        case 3: ringtone = "Party Complex"; break;
        case 4: ringtone = "Salty Ditty"; break;
    }

    TimerItem* item = new TimerItem(ui->newTimerName->text(), ui->newTimerBox->time().msecsSinceStartOfDay() / 1000, ringtone, this);
    ui->timersLayout->addWidget(item);
    connect(item, SIGNAL(elapsed(QString,QString)), this, SLOT(timerElapsed(QString,QString)));
    this->setCurrentWidget(ui->timersList);
    timersCreated++;
}

void TimerPage::timerElapsed(QString timerName, QString ringtone) {
    timersElapsed.append(timerName);

    if (timersElapsed.count() > 1) {
        QVariantMap hints;
        hints.insert("x-thesuite-timercomplete", true);
        hints.insert("suppress-sound", true);

        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(notificationInterface->asyncCall("Notify", "theShell", currentTimerId, "", tr("%n timers elapsed", nullptr, timersElapsed.count()), timersElapsed.join(" · "), QStringList(), hints, 0));
        connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
            currentTimerId = watcher->reply().arguments().first().toUInt();
        });
    } else {
        QStringList actions;
        actions << "restart" << "Restart Timer";
        actions << "+0.5" << "+30 sec";
        actions << "+1" << "+1 min";
        actions << "+2" << "+2 min";
        actions << "+5" << "+5 min";
        actions << "+10" << "+10 min";

        QVariantMap hints;
        hints.insert("x-thesuite-timercomplete", true);
        hints.insert("suppress-sound", true);

        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(notificationInterface->asyncCall("Notify", "theShell", currentTimerId, "", timerName, tr("Time's up!"), actions, hints, 0));
        connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
            currentTimerId = watcher->reply().arguments().first().toUInt();
        });


        QMediaPlaylist* playlist = new QMediaPlaylist();

        #ifdef BLUEPRINT
            QString ringtonesPath = "/usr/share/sounds/theshellb/tones/";
        #else
            QString ringtonesPath = "/usr/share/sounds/theshell/tones/";
        #endif

        if (ringtone == "Happy Bee") {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "happybee.ogg")));
        } else if (ringtone == "Playing in the Dark") {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "playinginthedark.ogg")));
        } else if (ringtone == "Ice Cream Truck") {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "icecream.ogg")));
        } else if (ringtone == "Party Complex") {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "party.ogg")));
        } else if (ringtone == "Salty Ditty") {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "saltyditty.ogg")));
        }
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
        this->ringtone->setPlaylist(playlist);
        this->ringtone->play();

        emit attenuate();
    }
}

void TimerPage::notificationClosed(uint id, uint reason) {
    if (id == currentTimerId) {
        currentTimerId = 0;
        timersElapsed.clear();
        ringtone->stop();
        emit deattenuate();
    }
}

void TimerPage::on_newTimerButtonTop_clicked()
{
    ui->newTimerButton->click();
}

void TimerPage::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
