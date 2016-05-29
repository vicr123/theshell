#include "segfaultdialog.h"
#include "ui_segfaultdialog.h"

SegfaultDialog::SegfaultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SegfaultDialog)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
}

SegfaultDialog::~SegfaultDialog()
{
    delete ui;
}

void SegfaultDialog::on_pushButton_clicked()
{
    this->close();
}
