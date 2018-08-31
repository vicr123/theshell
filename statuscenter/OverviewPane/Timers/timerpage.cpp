#include "timerpage.h"
#include "ui_timerpage.h"

TimerPage::TimerPage(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::TimerPage)
{
    ui->setupUi(this);
}

TimerPage::~TimerPage()
{
    delete ui;
}

void TimerPage::on_backButton_clicked()
{
    this->setCurrentWidget(ui->noTimersPage);
}

void TimerPage::on_newTimerButton_clicked()
{
    this->setCurrentWidget(ui->newTimerPage);
}
