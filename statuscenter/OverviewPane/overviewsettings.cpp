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

#include "overviewsettings.h"
#include "ui_overviewsettings.h"

#include <QCheckBox>
#include <QRadioButton>

OverviewSettings::OverviewSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewSettings)
{
    ui->setupUi(this);

    this->settingAttributes.providesLeftPane = false;
    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-overview");

    ui->weatherCheckBox->setChecked(settings.value("overview/enableWeather", false).toBool());
    if (settings.value("overview/weatherInCelsius", true).toBool()) {
        ui->celsiusRadio->setChecked(true);
    } else {
        ui->fahrenheitRadio->setChecked(true);
    }
}

OverviewSettings::~OverviewSettings()
{
    delete ui;
}

QWidget* OverviewSettings::mainWidget() {
    return this;
}

QString OverviewSettings::name() {
    return tr("Overview");
}

StatusCenterPaneObject::StatusPaneTypes OverviewSettings::type() {
    return Setting;
}

int OverviewSettings::position() {
    return 0;
}

void OverviewSettings::message(QString name, QVariantList args) {

}

void OverviewSettings::on_weatherCheckBox_toggled(bool checked)
{
    settings.setValue("overview/enableWeather", checked);
}

void OverviewSettings::on_celsiusRadio_toggled(bool checked)
{
    if (checked) {
        settings.setValue("overview/weatherInCelsius", true);
    }
}

void OverviewSettings::on_fahrenheitRadio_toggled(bool checked)
{
    if (checked) {
        settings.setValue("overview/weatherInCelsius", false);
    }
}

void OverviewSettings::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
