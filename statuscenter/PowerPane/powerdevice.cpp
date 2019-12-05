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
#include "powerdevice.h"
#include "ui_powerdevice.h"

#include <the-libs_global.h>
#include <UPower/desktopupowerdevice.h>
#include <hotkeyhud.h>

struct PowerDevicePrivate {
    DesktopUPowerDevice* device;
    QString icon;
};

PowerDevice::PowerDevice(DesktopUPowerDevice* device, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PowerDevice)
{
    ui->setupUi(this);

    d = new PowerDevicePrivate();
    d->device = device;

    connect(d->device, &DesktopUPowerDevice::propertiesUpdated, this, &PowerDevice::updateData);
    connect(d->device, &DesktopUPowerDevice::lowBatteryNotification, this, [=](QString description) {
        HotkeyHud::show({
            {"icon", QIcon::fromTheme(d->icon)},
            {"control", d->device->typeString()},
            {"value", d->device->percentage()},
            {"explanation", description},
            {"highlightCol", QColor(200, 0, 0)},
            {"timeout", 5000},
            {"sound", "battery-caution"}
        });
    });
    connect(d->device, &DesktopUPowerDevice::chargingNotification, this, [=] {
        HotkeyHud::show({
            {"icon", QIcon::fromTheme(d->icon)},
            {"control", d->device->typeString()},
            {"value", d->device->percentage()},
            {"explanation", tr("Charging")},
            {"highlightCol", QColor(0, 200, 0)},
            {"timeout", 5000},
            {"sound", "power-plug"}
        });
    });
    connect(d->device, &DesktopUPowerDevice::fullNotification, this, [=] {
        HotkeyHud::show({
            {"icon", QIcon::fromTheme(d->icon)},
            {"control", d->device->typeString()},
            {"value", d->device->percentage()},
            {"explanation", tr("Full")},
            {"highlightCol", QColor(0, 200, 0)},
            {"timeout", 5000},
//            {"sound", "battery-caution"} //TODO: a good sound :)
        });
    });
    connect(d->device, &DesktopUPowerDevice::dischargingNotification, this, [=] {
        HotkeyHud::show({
            {"icon", QIcon::fromTheme(d->icon)},
            {"control", d->device->typeString()},
            {"value", d->device->percentage()},
            {"explanation", tr("Discharging")},
            {"timeout", 5000},
//            {"sound", "battery-caution"} //TODO: a good sound :)
        });
    });
    this->updateData();
}

PowerDevice::~PowerDevice()
{
    delete ui;
}

DesktopUPowerDevice*PowerDevice::device()
{
    return d->device;
}

void PowerDevice::updateData()
{
    d->icon = d->device->iconName();

    ui->titleLabel->setText(d->device->typeString().toUpper());
    ui->iconLabel->setPixmap(QIcon::fromTheme(d->icon).pixmap(SC_DPI_T(QSize(32, 32), QSize)));

    switch (d->device->type()) {
        case DesktopUPowerDevice::LinePower:
            if (d->device->online()) {
                ui->statusLabel->setText(tr("Connected"));
            } else {
                ui->statusLabel->setText(tr("Disconnected"));
            }
            ui->percentageWidget->setVisible(false);
            break;
        case DesktopUPowerDevice::Battery:
            ui->statusLabel->setText(d->device->stateString());
            ui->percentageWidget->setVisible(true);
            ui->percentageBar->setValue(d->device->percentage());
            ui->percentageLabel->setText(QStringLiteral("%1%").arg(d->device->percentage()));
            break;
        case DesktopUPowerDevice::UnknownType:
        case DesktopUPowerDevice::Ups:
        case DesktopUPowerDevice::Monitor:
        case DesktopUPowerDevice::Mouse:
        case DesktopUPowerDevice::Keyboard:
        case DesktopUPowerDevice::Pda:
        case DesktopUPowerDevice::Phone:
            ui->statusLabel->setText(tr("Power Device"));
            break;

    }
}
