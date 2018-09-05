#include "reminderspage.h"
#include "ui_reminderspage.h"

RemindersPage::RemindersPage(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::RemindersPage)
{
    ui->setupUi(this);
}

RemindersPage::~RemindersPage()
{
    delete ui;
}
