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
#include "chunkwidget.h"
#include "ui_chunkwidget.h"

#include <the-libs_global.h>
#include <QIcon>

#include <BluezQt/Device>

ChunkWidget::ChunkWidget(BluezQt::Manager* mgr, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChunkWidget)
{
    ui->setupUi(this);

    ui->icon->setPixmap(QIcon::fromTheme("bluetooth").pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
    this->setVisible(false);
    this->mgr = mgr;

    connect(mgr, &BluezQt::Manager::deviceChanged, [=](BluezQt::DevicePtr device) {
        int connectedDevices = 0;
        QString deviceName;
        for (BluezQt::DevicePtr device : mgr->devices()) {
            if (device->isConnected()) {
                deviceName = device->name();
                connectedDevices++;
            }
        }

        if (connectedDevices == 0) {
            this->setVisible(false);
        } else if (connectedDevices == 1) {
            ui->devices->setText(deviceName);
            this->setVisible(true);
        } else {
            ui->devices->setText(tr("%n devices", nullptr, connectedDevices));
            this->setVisible(true);
        }
    });
    this->setVisible(false);
}

ChunkWidget::~ChunkWidget()
{
    delete ui;
}

void ChunkWidget::mousePressEvent(QMouseEvent *event) {
    emit showBluetoothPane();
}
