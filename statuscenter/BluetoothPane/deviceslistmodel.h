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

#ifndef DEVICESLISTMODEL_H
#define DEVICESLISTMODEL_H

#include <debuginformationcollector.h>
#include <QAbstractListModel>
#include <BluezQt/Manager>
#include <BluezQt/Device>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QListView>
#include <QApplication>
#include <QDesktopWidget>

class DevicesListModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        enum ShowDevice {
            Paired = 0x1,
            Connected = 0x2,
            Unpaired = 0x4,
            Unknown = 0x8
        };
        Q_DECLARE_FLAGS(ShowDevices, ShowDevice)

        explicit DevicesListModel(BluezQt::Manager* mgr, ShowDevices flags = ShowDevices(Paired | Connected), QObject *parent = T_QOBJECT_ROOT);

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    private slots:
        void addDevice(BluezQt::DevicePtr device);

    private:
        BluezQt::Manager* mgr;
        ShowDevices flags;
        QList<BluezQt::DevicePtr> showingDevices;
};

class DevicesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        DevicesDelegate(QWidget *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


#endif // DEVICESLISTMODEL_H
