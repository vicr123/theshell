#include "timeritem.h"
#include "ui_timeritem.h"

#include <QTime>
#include <tnotification.h>

TimerItem::TimerItem(QString timerName, int seconds, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimerItem)
{
    ui->setupUi(this);

    ui->timerName->setText(timerName);

    progressAnim = new tVariantAnimation();
    progressAnim->setForceAnimation(true);
    progressAnim->setStartValue(0);
    progressAnim->setEndValue(this->width());
    progressAnim->setDuration(seconds * 1000);
    connect(progressAnim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        int msecsElapsed = progressAnim->currentTime();

        QTime time = QTime::fromMSecsSinceStartOfDay(progressAnim->duration() - msecsElapsed);

        ui->timeLeftLabel->setText(time.toString("HH:mm:ss"));
        this->update();
    });
    connect(progressAnim, &tVariantAnimation::finished, [=] {
        emit elapsed(timerName);
        //if (AudioMan->QuietMode() != AudioManager::notifications && AudioMan->QuietMode() != AudioManager::mute) { //Check if we should show the notification so the user isn't stuck listening to the tone

            /*tNotification* notification = new tNotification(timerName, tr("Time's up!"));

            QStringList actions;
            actions << "restart" << "Restart Timer";
            actions << "+0.5" << "+30 sec";
            actions << "+1" << "+1 min";
            actions << "+2" << "+2 min";
            actions << "+5" << "+5 min";
            actions << "+10" << "+10 min";

            notification->insertHint("x-thesuite-timercomplete", true);
            notification->setSoundOn(false);

            /*QVariantMap hints;
            hints.insert("x-thesuite-timercomplete", true);
            hints.insert("suppress-sound", true);
            timerNotificationId = ndbus->Notify("theShell", 0, "", tr("Timer Elapsed"),
                                      tr("Your timer has completed."),
                                      actions, hints, 0);*/

            //notification->setTimeout(0);
            //notification->post();

            /*QMediaPlaylist* playlist = new QMediaPlaylist();

            #ifdef BLUEPRINT
                QString ringtonesPath = "/usr/share/sounds/theshellb/tones/";
            #else
                QString ringtonesPath = "/usr/share/sounds/theshell/tones/";
            #endif

            /*if (ui->timerToneSelect->currentText() == tr("Happy Bee")) {
                playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "happybee.ogg")));
            } else if (ui->timerToneSelect->currentText() == tr("Playing in the Dark")) {
                playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "playinginthedark.ogg")));
            } else if (ui->timerToneSelect->currentText() == tr("Ice Cream Truck")) {
                playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "icecream.ogg")));
            } else if (ui->timerToneSelect->currentText() == tr("Party Complex")) {
                playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "party.ogg")));
            } else if (ui->timerToneSelect->currentText() == tr("Salty Ditty")) {
                playlist->addMedia(QMediaContent(QUrl::fromLocalFile(ringtonesPath + "saltyditty.ogg")));
            }
            playlist->setPlaybackMode(QMediaPlaylist::Loop);
            ringtone->setPlaylist(playlist);
            ringtone->play();*/

            //AudioMan->attenuateStreams();
        //}
    });
    progressAnim->start();
}

TimerItem::~TimerItem()
{
    delete ui;
}

void TimerItem::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setBrush(this->palette().brush(QPalette::Highlight));
    painter.setPen(Qt::transparent);
    painter.drawRect(0, this->height() - 2 * theLibsGlobal::instance()->getDPIScaling(), progressAnim->currentValue().toInt(), 2 * theLibsGlobal::instance()->getDPIScaling());
}

void TimerItem::resizeEvent(QResizeEvent* event) {
    progressAnim->setEndValue(this->width());
}
