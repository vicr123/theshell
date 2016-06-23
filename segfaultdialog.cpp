#include "segfaultdialog.h"
#include "ui_segfaultdialog.h"

SegfaultDialog::SegfaultDialog(QString signal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SegfaultDialog)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    ui->label_3->setText("To debug, attach a debugger to PID " + QString::number(QApplication::applicationPid()));
    ui->signal->setText("Signal: " + signal);
}

SegfaultDialog::~SegfaultDialog()
{
    delete ui;
}

void SegfaultDialog::on_pushButton_clicked()
{
    this->close();
}
