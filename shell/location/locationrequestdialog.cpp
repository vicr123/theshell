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

#include "locationrequestdialog.h"
#include "ui_locationrequestdialog.h"

extern float getDPIScaling();

LocationRequestDialog::LocationRequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationRequestDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    ui->iconLabel->setPixmap(QIcon::fromTheme("gps").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->geoclueReason->setVisible(false);
}

LocationRequestDialog::~LocationRequestDialog()
{
    delete ui;
}

void LocationRequestDialog::on_denyButton_clicked()
{
    this->reject();
}

void LocationRequestDialog::setAppName(QString appName) {
    ui->requestMessage->setText(tr("Allow <b>%1</b> to use your physical location?").arg(appName));
    this->appName = appName;
}

void LocationRequestDialog::setIcon(QIcon icon) {
    ui->iconLabel->setPixmap(icon.pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
}

void LocationRequestDialog::on_allowButton_clicked()
{
    this->accept();
}

void LocationRequestDialog::setReason(QString reason) {
    ui->geoclueReason->setVisible(true);
    //ui->geoclueReason->setText(tr("%1 says \"%2\"").arg(appName, reason));
    ui->geoclueReason->setText(reason);
}
