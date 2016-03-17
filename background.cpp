#include "background.h"
#include "ui_background.h"

Background::Background(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Background)
{
    ui->setupUi(this);

    ui->label->setPixmap(QPixmap("/usr/share/icons/theos/backgrounds/triangle/1680x1050.png"));
}

Background::~Background()
{
    delete ui;
}
