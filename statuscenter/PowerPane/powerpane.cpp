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
#include "powerpane.h"
#include "ui_powerpane.h"

#include <the-libs_global.h>
#include <UPower/desktopupower.h>
#include <UPower/desktopupowerdevice.h>
#include "powerchunk.h"
#include "powerdevice.h"
#include <QTimer>
#include <powerdaemon.h>

struct PowerPanePrivate {
    DesktopUPower* daemon;
    QList<PowerDevice*> panes;

    PowerChunk* chunk;
};

PowerPane::PowerPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PowerPane)
{
    ui->setupUi(this);

    d = new PowerPanePrivate();

    d->daemon = new DesktopUPower();
    connect(d->daemon, QOverload<DesktopUPowerDevice*>::of(&DesktopUPower::deviceAdded), this, [=](DesktopUPowerDevice* device) {
        PowerDevice* dev = new PowerDevice(device);
        ui->devicesLayout->addWidget(dev);
        d->panes.append(dev);
    });
    connect(d->daemon, QOverload<DesktopUPowerDevice*>::of(&DesktopUPower::deviceRemoved), this, [=](DesktopUPowerDevice* device) {
        for (PowerDevice* dev : d->panes) {
            if (dev->device() == device) {
                ui->devicesLayout->removeWidget(dev);
                d->panes.removeAll(dev);
                dev->deleteLater();
                return;
            }
        }
    });

    this->informationalAttributes.lightColor = QColor(200, 150, 0);
    this->informationalAttributes.darkColor = QColor(100, 50, 0);

    connect(PowerDaemon::instance(), &PowerDaemon::powerStretchChanged, this, [=](bool isOn) {
        QSignalBlocker blocker(ui->powerStretchSwitch);
        ui->powerStretchSwitch->setChecked(isOn);
    });

    QTimer::singleShot(0, [=] {
        d->chunk = new PowerChunk(d->daemon);
        connect(d->chunk, &PowerChunk::activated, this, [=] {
            sendMessage("show", {});
        });

        sendMessage("register-chunk", {QVariant::fromValue(d->chunk)});
        sendMessage("register-snack", {QVariant::fromValue(d->chunk->snackWidget())});
    });
}

PowerPane::~PowerPane()
{
    d->daemon->deleteLater();

    delete d;
    delete ui;
}

QWidget*PowerPane::mainWidget()
{
    return this;
}

QString PowerPane::name()
{
    return tr("Power");
}

StatusCenterPaneObject::StatusPaneTypes PowerPane::type()
{
    return StatusCenterPaneObject::Informational;
}

int PowerPane::position()
{
    return 200;
}

void PowerPane::message(QString name, QVariantList args)
{

}

void PowerPane::updateShownDevices()
{
    QList<DesktopUPowerDevice*> newDevices;
}

void PowerPane::on_powerStretchSwitch_toggled(bool checked)
{
    PowerDaemon::instance()->setPowerStretch(checked);
}
