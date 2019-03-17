/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "jobviewwidget.h"
#include "ui_jobviewwidget.h"

#include "jobdbus.h"

JobViewWidget::JobViewWidget(QString title, QString icon, int capabilities, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::JobViewWidget)
{
    ui->setupUi(this);

    ui->appName->setText(title);

    ui->cancelButton->setEnabled(capabilities & JobDBus::Killable);
    ui->pauseButton->setVisible(capabilities & JobDBus::Suspendable);


    QIcon appIc = QIcon::fromTheme("generic-app");
    if (QIcon::hasThemeIcon(icon)) {
        appIc = QIcon::fromTheme(icon);
    } else if (QIcon::hasThemeIcon(title.toLower().replace(" ", "-"))) {
        appIc = QIcon::fromTheme(title.toLower().replace(" ", "-"));
    } else if (QIcon::hasThemeIcon(title.toLower().replace(" ", ""))) {
        appIc = QIcon::fromTheme(title.toLower().replace(" ", ""));
    } else {
        appIc = QIcon::fromTheme("generic-app");
    }

    ui->appIcon->setPixmap(appIc.pixmap(24, 24));

    ui->description1Container->setVisible(false);
    ui->description2Container->setVisible(false);
}

JobViewWidget::~JobViewWidget()
{
    delete ui;
}

void JobViewWidget::on_pauseButton_clicked()
{
    emit suspend();
}

void JobViewWidget::on_cancelButton_clicked()
{
    emit terminate();
    ui->cancelButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
}

void JobViewWidget::setSuspended(bool suspended) {

}

void JobViewWidget::setInfoMessage(QString message) {
    ui->infoTextLabel->setText(message.toUpper());
}

void JobViewWidget::setDescriptionField(uint number, QString name, QString value) {
    if (number == 1) {
        ui->description1Title->setText(name.toUpper());
        ui->description1->setText(value);
        ui->description1Container->setVisible(true);
    } else if (number == 2) {
        ui->description2Title->setText(name.toUpper());
        ui->description2->setText(value);
        ui->description2Container->setVisible(true);
    }
}

void JobViewWidget::clearDescriptionField(uint number) {
    if (number == 1) {
        ui->description1Container->setVisible(false);
    } else if (number == 2) {
        ui->description2Container->setVisible(false);
    }
}

void JobViewWidget::setPercent(uint percent) {
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(percent);
}
