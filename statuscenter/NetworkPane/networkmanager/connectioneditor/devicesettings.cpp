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
#include "devicesettings.h"
#include "ui_devicesettings.h"

#include <QMouseEvent>
#include <QDBusObjectPath>
#include <NetworkManagerQt/Device>
#include "devicesettingsmodel.h"

struct DeviceSettingsPrivate {
    QSharedPointer<NetworkManager::Device> device;
    DeviceSettingsModel* model;
};

DeviceSettings::DeviceSettings(QDBusObjectPath device, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceSettings)
{
    ui->setupUi(this);
    d = new DeviceSettingsPrivate();

    ui->leftPane->setFixedWidth(SC_DPI(300));
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    d->device.reset(new NetworkManager::Device(device.path()));
    switch (d->device->type()) {
        case NetworkManager::Device::Ethernet:
            ui->titleLabel->setText(tr("Wired Connection"));
            break;
        case NetworkManager::Device::Wifi:
            ui->titleLabel->setText(tr("WiFi"));
            break;
        case NetworkManager::Device::Bluetooth:
            ui->titleLabel->setText(tr("Bluetooth"));
            break;
        case NetworkManager::Device::Modem:
            ui->titleLabel->setText(tr("Cellular"));
            break;
        case NetworkManager::Device::UnknownType:
        default:
            ui->titleLabel->setText(tr("Networking Device"));
            break;
    }

    d->model = new DeviceSettingsModel(d->device, this);
    ui->connectionsWidget->setModel(d->model);
    ui->connectionsWidget->setItemDelegate(new DeviceSettingsDelegate());

    connect(d->model, &DeviceSettingsModel::closeDialog, this, &DeviceSettings::close);
    connect(d->model, &DeviceSettingsModel::showWidget, this, [=](QWidget* widget) {
        if (ui->stackedWidget->indexOf(widget) == -1) {
            ui->stackedWidget->addWidget(widget);
        }
        ui->stackedWidget->setCurrentWidget(widget);
    });

    ui->connectionsWidget->viewport()->installEventFilter(this);
}

DeviceSettings::~DeviceSettings()
{
    delete d;
    delete ui;
}

void DeviceSettings::on_backButton_clicked()
{
    close();
}

void DeviceSettings::close() {
    emit done();
}

void DeviceSettings::on_connectionsWidget_clicked(const QModelIndex &index)
{

}

bool DeviceSettings::eventFilter(QObject *watched, QEvent *event) {

    if (event->type() == QEvent::MouseButtonRelease && watched == ui->connectionsWidget->viewport()) {
        QMouseEvent *e = (QMouseEvent*) event;
        QModelIndex index = ui->connectionsWidget->indexAt(e->pos());
        if (!index.isValid()) {
            return false;
        }

        QSize size = ui->connectionsWidget->sizeHintForIndex(index);
        if (QApplication::layoutDirection() == Qt::RightToLeft
                ? (e->pos().x() < size.height())
                : (e->pos().x() > ui->connectionsWidget->width() - size.height())) {
            //We're positioned over the secondary item
            d->model->activateSecondary(index);
        } else {
            d->model->activateItem(index);
        }
        e->ignore();
        return true;
    }
    return false;
}
