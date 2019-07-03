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
#ifndef DEVICESETTINGSMODEL_H
#define DEVICESETTINGSMODEL_H

#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <NetworkManagerQt/Device>

struct DeviceSettingsModelPrivate;
class DeviceSettingsModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit DeviceSettingsModel(QSharedPointer<NetworkManager::Device> device, QObject *parent = nullptr);
        ~DeviceSettingsModel() override;

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QWidget* configurationWidgetForIndex(QModelIndex index);

    public slots:
        void activateItem(QModelIndex index);
        void activateSecondary(QModelIndex index);

    signals:
        void closeDialog();
        void showWidget(QWidget* widget);

    private:
        DeviceSettingsModelPrivate* d;

        void addConnection(NetworkManager::Connection::Ptr connection, int index = -1);
};

class DeviceSettingsDelegate : public QStyledItemDelegate
{
        Q_OBJECT

    public:
        explicit DeviceSettingsDelegate(QObject* parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DEVICESETTINGSMODEL_H
