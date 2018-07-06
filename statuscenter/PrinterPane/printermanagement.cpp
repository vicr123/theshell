#include "printermanagement.h"
#include "ui_printermanagement.h"

PrinterManagement::PrinterManagement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrinterManagement)
{
    ui->setupUi(this);

    this->settingAttributes.icon = QIcon::fromTheme("printer");
    this->settingAttributes.menuWidget = ui->menuWidget;

    destCount = cupsGetDests(&dests);
    for (int i = 0; i < destCount; i++) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QString::fromLocal8Bit(dests[i].name));
        item->setIcon(QIcon::fromTheme("printer"));
        ui->printerList->addItem(item);
    }
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

void PrinterManagement::message(QString name, QVariantList args) {

}

void PrinterManagement::on_mainMenuButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}
