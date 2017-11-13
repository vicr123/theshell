/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#include "choosebackground.h"
#include "ui_choosebackground.h"

ChooseBackground::ChooseBackground(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseBackground)
{
    ui->setupUi(this);

    QString backPath = settings.value("desktop/background", "inbuilt:triangles").toString();

    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/triangles"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/ribbon"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/blueprint"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/triplecircle"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/shatter"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/slice"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/nav"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/leftwaves"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/beach"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/arrows"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/waves"), ""));

    if (backPath.startsWith("inbuilt:")) { //Inbuilt background
        ui->radioButton->setChecked(true);
        QString selection = ":/backgrounds/" + backPath.split(":").at(1);
        if (selection == "triangles") {
            ui->listWidget->item(0)->setSelected(true);
        } else if (selection == "ribbon") {
            ui->listWidget->item(1)->setSelected(true);
        } else if (selection == "blueprint") {
            ui->listWidget->item(2)->setSelected(true);
        } else if (selection == "triplecircle") {
            ui->listWidget->item(3)->setSelected(true);
        } else if (selection == "shatter") {
            ui->listWidget->item(4)->setSelected(true);
        } else if (selection == "slice") {
            ui->listWidget->item(5)->setSelected(true);
        } else if (selection == "nav") {
            ui->listWidget->item(6)->setSelected(true);
        } else if (selection == "leftwaves") {
            ui->listWidget->item(7)->setSelected(true);
        } else if (selection == "beach") {
            ui->listWidget->item(8)->setSelected(true);
        } else if (selection == "arrows") {
            ui->listWidget->item(9)->setSelected(true);
        } else if (selection == "waves") {
            ui->listWidget->item(10)->setSelected(true);
        }
        ui->lineEdit->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->radioButton_2->setChecked(true);
        ui->lineEdit->setText(backPath);
        ui->listWidget->setEnabled(false);
    }

}

ChooseBackground::~ChooseBackground()
{
    delete ui;
}

QIcon ChooseBackground::getSvgIcon(QString filename) {
    QPixmap background(150, 84);
    QSvgRenderer renderer(filename);
    QPainter painter(&background);
    renderer.render(&painter, background.rect());
    return QIcon(background);
}

void ChooseBackground::on_lineEdit_textChanged(const QString &arg1)
{
    settings.setValue("desktop/background", arg1);
    emit reloadBackgrounds();
    this->setFocus();
}

void ChooseBackground::on_radioButton_2_toggled(bool checked)
{
    if (checked) {
        ui->lineEdit->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    } else {
        ui->lineEdit->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
}

void ChooseBackground::on_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->listWidget->setEnabled(true);
    } else {
        ui->listWidget->setEnabled(false);
    }
}

void ChooseBackground::on_listWidget_currentRowChanged(int currentRow)
{
    switch (currentRow) {
        case 0:
            settings.setValue("desktop/background", "inbuilt:triangles");
            break;
        case 1:
            settings.setValue("desktop/background", "inbuilt:ribbon");
            break;
        case 2:
            settings.setValue("desktop/background", "inbuilt:blueprint");
            break;
        case 3:
            settings.setValue("desktop/background", "inbuilt:triplecircle");
            break;
        case 4:
            settings.setValue("desktop/background", "inbuilt:shatter");
            break;
        case 5:
            settings.setValue("desktop/background", "inbuilt:slice");
            break;
        case 6:
            settings.setValue("desktop/background", "inbuilt:nav");
            break;
        case 7:
            settings.setValue("desktop/background", "inbuilt:leftwaves");
            break;
        case 8:
            settings.setValue("desktop/background", "inbuilt:beach");
            break;
        case 9:
            settings.setValue("desktop/background", "inbuilt:arrows");
            break;
        case 10:
            settings.setValue("desktop/background", "inbuilt:waves");
            break;
    }
    emit reloadBackgrounds();
    this->setFocus();
}

void ChooseBackground::on_pushButton_clicked()
{
    delete this;
}

void ChooseBackground::on_pushButton_2_clicked()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(this, tr("Select Background"), "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
}
