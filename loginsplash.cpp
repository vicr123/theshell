#include "loginsplash.h"
#include "ui_loginsplash.h"

LoginSplash::LoginSplash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginSplash)
{
    ui->setupUi(this);

    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(close()));
    timer->start();
}

LoginSplash::~LoginSplash()
{
    delete ui;
}
