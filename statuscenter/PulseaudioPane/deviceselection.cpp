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
#include "deviceselection.h"
#include "ui_deviceselection.h"

#include <Context>
#include <Sink>

DeviceSelection::DeviceSelection(DeviceSelection::DeviceType type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceSelection)
{
    ui->setupUi(this);

    QVector<PulseAudioQt::Device*> devices;
    switch (type) {
        case Sink:
            ui->titleLabel->setText(tr("Select Output Device"));
            ui->descriptionLabel->setText(tr("Which device do you want this sound source to be played on?"));
            for (PulseAudioQt::Sink* sink : PulseAudioQt::Context::instance()->sinks()) {
                devices.append(sink);
            }
            break;
    }

    for (PulseAudioQt::Device* device : devices) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(device->description());
        item->setData(Qt::UserRole, device->index());
        ui->deviceList->addItem(item);
    }
}

DeviceSelection::~DeviceSelection()
{
    delete ui;
}

void DeviceSelection::on_backButton_clicked()
{
    emit rejected();
}

void DeviceSelection::on_deviceList_itemActivated(QListWidgetItem *item)
{
    emit accepted(item->data(Qt::UserRole).toUInt());
}
