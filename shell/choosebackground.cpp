/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

ChooseBackground::ChooseBackground(QString currentCommunityBackground, QWidget *parent) :
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
        ui->inbuiltBackground->setChecked(true);
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
    } else if (backPath.startsWith("community")) {
        ui->community->setChecked(true);
    } else {
        ui->custom->setChecked(true);
        ui->lineEdit->setText(backPath);
    }

    ui->waitTime->setValue(settings.value("desktop/waitTime", 30).toInt());
    ui->showLabels->setChecked(settings.value("desktop/showLabels", true).toBool());

    if (currentCommunityBackground != "") {
        QFile metadataFile(QDir::homePath() + "/.theshell/backgrounds/" + currentCommunityBackground + "/metadata.json");
        metadataFile.open(QFile::ReadOnly);
        QJsonDocument doc = QJsonDocument::fromJson(metadataFile.readAll());
        metadataFile.close();

        QJsonObject obj = doc.object();
        ui->authorLabel->setText(tr("by %1").arg(obj.value("author").toString()));

        license = obj.value("copyright").toString();
        ui->license->setText(license);
    } else {
        ui->currentCommunityFrame->setVisible(false);
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

void ChooseBackground::on_inbuiltBackground_toggled(bool checked)
{
    if (checked) {
        ui->backgroundType->setCurrentIndex(0);
    }
}

void ChooseBackground::on_community_toggled(bool checked)
{
    if (checked) {
        ui->backgroundType->setCurrentIndex(1);

        settings.setValue("desktop/background", "community");
        emit reloadBackgrounds();
        this->setFocus();
    }
}

void ChooseBackground::on_custom_toggled(bool checked)
{
    if (checked) {
        ui->backgroundType->setCurrentIndex(2);
    }
}

void ChooseBackground::on_pushButton_3_clicked()
{

}

void ChooseBackground::on_waitTime_valueChanged(int arg1)
{
    settings.setValue("desktop/waitTime", arg1);
    emit reloadTimer();
}


void ChooseBackground::on_showLabels_toggled(bool checked)
{
    settings.setValue("desktop/showLabels", checked);
    emit reloadBackgrounds();
}

void ChooseBackground::on_licenseInfoButton_clicked()
{
    if (license == "CC BY-SA 4.0") {
        QProcess::startDetached("xdg-open \"https://creativecommons.org/licenses/by-sa/4.0/\"");
    } else if (license == "CC0") {
        QProcess::startDetached("xdg-open \"https://creativecommons.org/publicdomain/zero/1.0/\"");
    } else if (license == "WTFPL") {
        QProcess::startDetached("xdg-open \"http://www.wtfpl.net/about/\"");
    }
}
