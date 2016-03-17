#include "lockscreen.h"
#include "ui_lockscreen.h"

LockScreen::LockScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LockScreen)
{
    ui->setupUi(this);
}

LockScreen::~LockScreen()
{
    delete ui;
}
