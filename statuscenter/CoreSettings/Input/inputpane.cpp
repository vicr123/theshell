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
#include "inputpane.h"
#include "ui_inputpane.h"

InputPane::InputPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputPane)
{
    ui->setupUi(this);

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-input");
    this->settingAttributes.menuWidget = ui->LeftPaneWidget;
    this->settingAttributes.providesLeftPane = true;

    connect(ui->keyboardPane, &KeyboardPane::loadNewKeyboardLayoutMenu, this, [=] {
        sendMessage("reload-keyboard-layouts", QVariantList());
    });

    ui->settingsStack->setCurrentAnimation(tStackedWidget::Lift);
}

InputPane::~InputPane()
{
    delete ui;
}

QWidget* InputPane::mainWidget() {
    return this;
}

QString InputPane::name() {
    return tr("Input");
}

StatusCenterPaneObject::StatusPaneTypes InputPane::type() {
    return Setting;
}

int InputPane::position() {
    return 0;
}

void InputPane::message(QString name, QVariantList args) {

}

void InputPane::on_backButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}
