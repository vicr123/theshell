#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"

InfoPaneDropdown::InfoPaneDropdown(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    ui->setupUi(this);
}

InfoPaneDropdown::~InfoPaneDropdown()
{
    delete ui;
}

void InfoPaneDropdown::show(dropdownType showWith) {
    changeDropDown(showWith);
    QDialog::showFullScreen();
}

void InfoPaneDropdown::changeDropDown(dropdownType changeTo) {
    QFrame *currentDropDownFrame;
    this->currentDropDown = changeTo;
    switch (changeTo) {
    case Clock:
        ui->clockFrame->setVisible(true);
        ui->clockFrame->move(0, 0);
        break;
    case Battery:

    case Notifications:
        break;
    }
}

void InfoPaneDropdown::resizeEvent(QResizeEvent *event) {
    event->accept();
    QSize size = event->size();

    ui->clockFrame->resize(size);

}

void InfoPaneDropdown::on_pushButton_clicked()
{
    this->close();
}
