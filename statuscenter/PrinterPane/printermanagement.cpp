#include "printermanagement.h"
#include "ui_printermanagement.h"

PrinterManagement::PrinterManagement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrinterManagement)
{
    ui->setupUi(this);

    this->settingAttributes.icon = QIcon::fromTheme("printer");
}

PrinterManagement::~PrinterManagement()
{
    delete ui;
}

QString PrinterManagement::name() {
    return "Printers";
}

PrinterManagement::StatusPaneTypes PrinterManagement::type() {
    return Setting;
}

QWidget* PrinterManagement::mainWidget() {
    return this;
}

int PrinterManagement::position() {
    return 900;
}
