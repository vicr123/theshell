/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
