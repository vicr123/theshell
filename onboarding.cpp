#include "onboarding.h"
#include "ui_onboarding.h"

Onboarding::Onboarding(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Onboarding)
{
    ui->setupUi(this);

    ui->buttonBox->setVisible(false);
    ui->changelog->setText(ChangelogOnbording);
    ui->thewaveLogo->setPixmap(QIcon(":/icons/thewave.svg").pixmap(256, 256));
    ui->stackedWidget->setCurrentIndex(0);
}

Onboarding::~Onboarding()
{
    delete ui;
}

void Onboarding::on_closeButton_clicked()
{
    this->close();
}

void Onboarding::on_stackedWidget_currentChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
    ui->nextButton->setVisible(true);
    switch (arg1) {
    case 0:
    case 3:
        ui->buttonBox->setVisible(false);
        break;
    case 2:
        ui->nextButton->setVisible(false);
        break;
    }
}

void Onboarding::on_nextButtonFirstPage_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_nextButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
}

void Onboarding::on_beginButton_clicked()
{
    this->close();
}

void Onboarding::on_enabletheWaveButton_clicked()
{
    settings.setValue("thewave/enabled", true);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_disabletheWaveButton_clicked()
{
    settings.setValue("thewave/enabled", false);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}
