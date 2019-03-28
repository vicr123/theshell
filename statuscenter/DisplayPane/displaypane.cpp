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
#include "displaypane.h"
#include "ui_displaypane.h"

DisplayPane::DisplayPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayPane)
{
    ui->setupUi(this);

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-display");
}

DisplayPane::~DisplayPane()
{
    delete ui;
}

QWidget* DisplayPane::mainWidget() {
    return this;
}

QString DisplayPane::name() {
    return tr("Display");
}

StatusCenterPaneObject::StatusPaneTypes DisplayPane::type() {
    return Setting;
}

int DisplayPane::position() {
    return 0;
}

void DisplayPane::message(QString name, QVariantList args) {

}
