#include "stopwatchpage.h"
#include "ui_stopwatchpage.h"

StopwatchPage::StopwatchPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StopwatchPage)
{
    ui->setupUi(this);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, [=] {
        int msecs = startTime.msecsTo(QDateTime::currentDateTimeUtc()) + msecsPadding;
        QTime t = QTime::fromMSecsSinceStartOfDay(msecs);
        if (t.isValid()) {
            ui->stopwatchFace->setText(t.toString("HH:mm:ss:zzz"));
        } else {
            ui->stopwatchFace->setText(tr("Overflow"));
        }
    });
    timer->setInterval(100);
}

StopwatchPage::~StopwatchPage()
{
    delete ui;
}

void StopwatchPage::on_startButton_clicked()
{
    if (timer->isActive()) {
        ui->startButton->setText(tr("Start"));
        ui->startButton->setIcon(QIcon::fromTheme("chronometer-start"));
        timer->stop();
        msecsPadding = startTime.msecsTo(QDateTime::currentDateTimeUtc()) + msecsPadding;
    } else {
        startTime = QDateTime::currentDateTimeUtc();
        timer->start();
        ui->startButton->setText(tr("Pause"));
        ui->startButton->setIcon(QIcon::fromTheme("chronometer-pause"));
    }
}

void StopwatchPage::on_resetButton_clicked()
{
    timer->stop();
    ui->stopwatchFace->setText("00:00:00:000");
    ui->startButton->setText(tr("Start"));
    ui->startButton->setIcon(QIcon::fromTheme("chronometer-start"));
    msecsPadding = 0;
}
