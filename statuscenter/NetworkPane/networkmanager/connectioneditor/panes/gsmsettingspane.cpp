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
#include "gsmsettingspane.h"
#include "ui_gsmsettingspane.h"

#include <NetworkManagerQt/GsmSetting>

struct GsmSettingsPanePrivate {
    NetworkManager::GsmSetting::Ptr settings;
};

GsmSettingsPane::GsmSettingsPane(NetworkManager::Connection::Ptr connection, QWidget *parent) :
    SettingPane(connection, parent),
    ui(new Ui::GsmSettingsPane)
{
    ui->setupUi(this);

    d = new GsmSettingsPanePrivate();
}

GsmSettingsPane::~GsmSettingsPane()
{
    delete ui;
    delete d;
}

void GsmSettingsPane::updateFields() {
    d->settings = connection->settings()->setting(NetworkManager::Setting::Gsm).staticCast<NetworkManager::GsmSetting>();

    ui->numberField->setText(d->settings->number());
    ui->apnField->setText(d->settings->apn());
    ui->usernameField->setText(d->settings->username());
    ui->passwordField->setText(d->settings->password());
    ui->allowRoamingSwitch->setChecked(!d->settings->homeOnly());
}

void GsmSettingsPane::on_apnField_textChanged(const QString &arg1)
{
    d->settings->setApn(arg1);
    settingsChanged();
}

void GsmSettingsPane::on_usernameField_textChanged(const QString &arg1)
{
    d->settings->setUsername(arg1);
    settingsChanged();
}

void GsmSettingsPane::on_passwordField_textChanged(const QString &arg1)
{
    d->settings->setPassword(arg1);
    settingsChanged();
}

void GsmSettingsPane::on_allowRoamingSwitch_toggled(bool checked)
{
    d->settings->setHomeOnly(!checked);
    settingsChanged();
}

void GsmSettingsPane::on_numberField_textChanged(const QString &arg1)
{
    d->settings->setNumber(arg1);
    settingsChanged();
}
