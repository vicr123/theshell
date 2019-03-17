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

enum NmDeviceType {
    UnknownType = 0,
    Ethernet,
    Wifi,
    Unused1,
    Unused2,
    Bluetooth,
    OlpcMesh,
    Wimax,
    Modem,
    Infiniband,
    Bond,
    Vlan,
    Adsl,
    Bridge,
    Generic,
    Team,
    Tun,
    IpTunnel,
    MacVlan,
    Vxlan,
    Macsec,
    Dummy
};

enum NmDeviceState {
    UnknownState = 0,
    Unmanaged = 10,
    Unavailable = 20,
    Disconnected = 30,
    Prepare = 40,
    Config = 50,
    NeedAuth = 60,
    IpConfig = 70,
    IpCheck = 80,
    Secondaries = 90,
    Activated = 100,
    Deactivating = 110,
    Failed = 120,
};

enum NmDeviceCapabilities {
    NoneCapabilities = 0,
    Wep40Cipher = 0x1,
    Wep104Cipher = 0x2,
    TkipCipher = 0x4,
    CcmpCipher = 0x8,
    Wpa = 0x10,
    Rsh = 0x20,
    Ap = 0x40,
    Adhoc = 0x80,
    FreqValid = 0x100,
    Freq2Ghz = 0x200,
    Freq5Ghz = 0x400
};

enum NmApSecurityFlags {
    NoneSecurityFlags = 0,
    PairWep40 = 0x1,
    PairWep104 = 0x2,
    PairTkip = 0x4,
    PairCcmp = 0x8,
    GroupWep40 = 0x10,
    GroupWep104 = 0x20,
    GroupTkip = 0x40,
    GroupCcmp = 0x80,
    KeyMgmtPsk = 0x100,
    KeyMgmt8021X = 0x200
};

enum SecurityType {
    NoSecurity,
    Leap,
    StaticWep,
    DynamicWep,
    WpaPsk,
    Wpa2Psk,
    WpaEnterprise,
    Wpa2Enterprise
};

class AvailableNetworksList : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AvailableNetworksList(QDBusObjectPath devicePath, QObject *parent = nullptr);

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
