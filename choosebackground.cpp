#include "choosebackground.h"
#include "ui_choosebackground.h"

ChooseBackground::ChooseBackground(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseBackground)
{
    ui->setupUi(this);
}

ChooseBackground::~ChooseBackground()
{
    delete ui;
}
