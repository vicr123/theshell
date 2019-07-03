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

#ifndef AVAILABLENETWORKSLIST_H
#define AVAILABLENETWORKSLIST_H

#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QIcon>
#include <QPainter>
#include <QDBusArgument>
#include <QDBusPendingCall>
#include <QEventLoop>
#include <debuginformationcollector.h>
#include "enums.h"

class AvailableNetworksList : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AvailableNetworksList(QDBusObjectPath devicePath, QObject *parent = T_QOBJECT_ROOT);

    struct AccessPoint {
        NmApSecurityFlags WpaFlags, RsnFlags;
        QString ssid;
        int strength;
        QDBusObjectPath path;
        SecurityType security;
        bool known;
    };

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QDBusObjectPath devicePath();

private slots:
    void reloadAP();

private:
    QDBusInterface* deviceInterface;

    QList<AccessPoint> accessPoints;
    bool reloadingNetworks = false;
};

Q_DECLARE_METATYPE(AvailableNetworksList::AccessPoint)

class AvailableNetworksListDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit AvailableNetworksListDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // AVAILABLENETWORKSLIST_H
