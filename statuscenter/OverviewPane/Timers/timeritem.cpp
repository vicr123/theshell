#include "timeritem.h"
#include "ui_timeritem.h"

#include <QTime>
#include <QTimer>
#include <tnotification.h>

TimerItem::TimerItem(QString timerName, int seconds, QString ringtone, QWidget *parent) :
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
    connect(progressAnim, &tVariantAnimation::valueChanged, [=] {
        int msecsElapsed = progressAnim->currentTime();

        QTime time = QTime::fromMSecsSinceStartOfDay(progressAnim->duration() - msecsElapsed);

        ui->timeLeftLabel->setText(time.toString("HH:mm:ss"));
        this->update();
    });
    connect(progressAnim, &tVariantAnimation::finished, [=] {
        emit elapsed(timerName, ringtone);

        progressAnim->setCurrentTime(0);

        ui->pauseButton->setIcon(QIcon::fromTheme("media-playback-start"));

        QTime time = QTime::fromMSecsSinceStartOfDay(progressAnim->duration());
        ui->timeLeftLabel->setText(time.toString("HH:mm:ss"));
    });
    progressAnim->start();

    QTimer* timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [=] {
        int msecsElapsed = progressAnim->currentTime();

        QTime time = QTime::fromMSecsSinceStartOfDay(progressAnim->duration() - msecsElapsed);

        ui->timeLeftLabel->setText(time.toString("HH:mm:ss"));
    });
    timer->start();

    QTime time = QTime::fromMSecsSinceStartOfDay(progressAnim->duration());
    ui->timeLeftLabel->setText(time.toString("HH:mm:ss"));

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

void TimerItem::on_pauseButton_clicked()
{
    if (progressAnim->state() == tVariantAnimation::Running) {
        progressAnim->pause();
        ui->timeLeftLabel->setEnabled(false);
        ui->pauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    } else {
        progressAnim->start();
        ui->timeLeftLabel->setEnabled(true);
        ui->pauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    }
}
