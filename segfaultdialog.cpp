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

void SegfaultDialog::on_pushButton_2_clicked()
{
    void* buffer[50];
    int size = backtrace(buffer, 50);
    char** charBt = backtrace_symbols(buffer, size);
    QString trace;
    for (int i = 0; i < size && charBt != NULL; i++) {
        trace.append(QString(charBt[i]) + "\n");
    }
    QMessageBox::information(this, "Backtrace", trace, QMessageBox::Ok, QMessageBox::Ok);
}
