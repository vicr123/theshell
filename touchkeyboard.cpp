#include "touchkeyboard.h"
#include "ui_touchkeyboard.h"

TouchKeyboard::TouchKeyboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TouchKeyboard)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_ShowWithoutActivating);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    this->setGeometry(screenGeometry.left(), screenGeometry.top() + screenGeometry.height() * (3 / 4), screenGeometry.width(), screenGeometry.height() / 4);

    connect(ui->qButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->wButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->eButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->rButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->tButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->yButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->uButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->iButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->oButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->pButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->aButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->sButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->dButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->fButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->gButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->hButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->jButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->kButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->lButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->zButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->xButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->cButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->vButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->bButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->nButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
    connect(ui->mButton, SIGNAL(clicked(bool)), this, SLOT(sendKey()));
}

TouchKeyboard::~TouchKeyboard()
{
    delete ui;
}


void TouchKeyboard::on_shiftButton_toggled(bool checked)
{
    if (checked) {
        ui->qButton->setText("Q");
        ui->wButton->setText("W");
        ui->eButton->setText("E");
        ui->rButton->setText("R");
        ui->tButton->setText("T");
        ui->yButton->setText("Y");
        ui->uButton->setText("U");
        ui->iButton->setText("I");
        ui->oButton->setText("O");
        ui->pButton->setText("P");
        ui->aButton->setText("A");
        ui->sButton->setText("S");
        ui->dButton->setText("D");
        ui->fButton->setText("F");
        ui->gButton->setText("G");
        ui->hButton->setText("H");
        ui->jButton->setText("J");
        ui->kButton->setText("K");
        ui->lButton->setText("L");
        ui->zButton->setText("Z");
        ui->xButton->setText("X");
        ui->cButton->setText("C");
        ui->vButton->setText("V");
        ui->bButton->setText("B");
        ui->nButton->setText("N");
        ui->mButton->setText("M");
    } else {
        ui->qButton->setText("q");
        ui->wButton->setText("w");
        ui->eButton->setText("e");
        ui->rButton->setText("r");
        ui->tButton->setText("t");
        ui->yButton->setText("y");
        ui->uButton->setText("u");
        ui->iButton->setText("i");
        ui->oButton->setText("o");
        ui->pButton->setText("p");
        ui->aButton->setText("a");
        ui->sButton->setText("s");
        ui->dButton->setText("d");
        ui->fButton->setText("f");
        ui->gButton->setText("g");
        ui->hButton->setText("h");
        ui->jButton->setText("j");
        ui->kButton->setText("k");
        ui->lButton->setText("l");
        ui->zButton->setText("z");
        ui->xButton->setText("x");
        ui->cButton->setText("c");
        ui->vButton->setText("v");
        ui->bButton->setText("b");
        ui->nButton->setText("n");
        ui->mButton->setText("m");
    }
}

void TouchKeyboard::sendKey() {
    QString text = ((QPushButton*) sender())->text();


}
