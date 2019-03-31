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
#include "settingspane.h"
#include "ui_settingspane.h"

#include "applicationnotificationmodel.h"

struct SettingsPanePrivate {
    NotificationsPermissionEngine currentSettings;
};

SettingsPane::SettingsPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPane)
{
    ui->setupUi(this);
    d = new SettingsPanePrivate();

    this->settingAttributes.icon = QIcon::fromTheme("preferences-system-notifications");
    this->settingAttributes.menuWidget = ui->LeftPaneWidget;
    this->settingAttributes.providesLeftPane = true;

    ui->appList->setModel(new ApplicationNotificationModel());
    ui->appList->setItemDelegate(new ApplicationNotificationModelDelegate());
    ui->appList->setIconSize(QSize(32, 32) * theLibsGlobal::getDPIScaling());
    connect(ui->appList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](QModelIndex current, QModelIndex previous) {
        if (current.row() == 0) {
            ui->stackedWidget->setCurrentIndex(0);
        } else if (current.row() == 1) {
            //don't do anything
        } else {
            ui->stackedWidget->setCurrentIndex(1);
            ApplicationInformation info = current.data(Qt::UserRole).value<ApplicationInformation>();
            d->currentSettings = info.permissionsEngine();
        }
    });
}

SettingsPane::~SettingsPane()
{
    delete d;
    delete ui;
}

QWidget* SettingsPane::mainWidget() {
    return this;
}

QString SettingsPane::name() {
    return tr("Notifications");
}

StatusCenterPaneObject::StatusPaneTypes SettingsPane::type() {
    return Setting;
}

int SettingsPane::position() {
    return 0;
}

void SettingsPane::message(QString name, QVariantList args) {

}


void SettingsPane::on_backButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}
