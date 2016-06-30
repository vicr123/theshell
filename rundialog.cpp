#include "rundialog.h"
#include "ui_rundialog.h"

RunDialog::RunDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunDialog)
{
    ui->setupUi(this);
}

RunDialog::~RunDialog()
{
    delete ui;
}

void RunDialog::on_cancelButton_clicked()
{
    delete this;
}

void RunDialog::on_runButton_clicked()
{
    QProcess::startDetached(ui->command->text());
    delete this;
}

void RunDialog::on_command_returnPressed()
{
    ui->runButton->click();
}
