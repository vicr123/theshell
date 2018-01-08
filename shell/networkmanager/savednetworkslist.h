/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#ifndef SAVEDNETWORKSLIST_H
#define SAVEDNETWORKSLIST_H

#include <QAbstractListModel>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusArgument>
#include <QIcon>

class Setting {
    public:
        enum type {
            Wired,
            Wireless,
            Bluetooth,
            Mobile,
            VPN,
            Unknown
        };

        void del();

        QDBusObjectPath path;
        QString name;
        type NetworkType;
};
Q_DECLARE_METATYPE(Setting)

class SavedNetworksList : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit SavedNetworksList(QObject *parent = nullptr);

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    public slots:
        void loadNetworks();

    private:
        QDBusInterface* savedInterface;
        QList<Setting> savedNetworks;
};

#endif // SAVEDNETWORKSLIST_H
