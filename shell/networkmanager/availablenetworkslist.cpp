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

#include "availablenetworkslist.h"

extern float getDPIScaling();

AvailableNetworksList::AvailableNetworksList(QDBusObjectPath devicePath, QObject *parent)
    : QAbstractListModel(parent)
{
    deviceInterface = new QDBusInterface("org.freedesktop.NetworkManager", devicePath.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus());
    QDBusConnection::systemBus().connect(deviceInterface->service(), devicePath.path(), deviceInterface->interface(), "AccessPointAdded", this, SLOT(reloadAP()));
    QDBusConnection::systemBus().connect(deviceInterface->service(), devicePath.path(), deviceInterface->interface(), "AccessPointRemoved", this, SLOT(reloadAP()));

    deviceInterface->call("RequestScan", QVariantMap());

    reloadAP();
}

int AvailableNetworksList::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return accessPoints.count();
}

void AvailableNetworksList::reloadAP() {
    if (reloadingNetworks) {
        return;
    }
    reloadingNetworks = true;

    int capabilities = deviceInterface->property("WirelessCapabilities").toInt();
    QList<QDBusObjectPath> ap = deviceInterface->property("AccessPoints").value<QList<QDBusObjectPath>>();

    QStringList knownSsids;

    QList<AccessPoint> accessPoints;

    for (QDBusObjectPath p : ap) {
        QDBusInterface apInterface(deviceInterface->service(), p.path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus());

        QString ssid = apInterface.property("Ssid").toString();
        if (!knownSsids.contains(ssid) && ssid != "") {
            knownSsids.append(ssid);

            AccessPoint accessPoint;
            accessPoint.ssid = ssid;
            accessPoint.strength = apInterface.property("Strength").toInt();
            accessPoint.WpaFlags = (NmApSecurityFlags) apInterface.property("WpaFlags").toInt();
            accessPoint.RsnFlags = (NmApSecurityFlags) apInterface.property("RsnFlags").toInt();
            accessPoint.path = p;
            accessPoint.known = false;

            if ((accessPoint.RsnFlags & PairTkip) && (capabilities & TkipCipher)) {
                if (accessPoint.WpaFlags & KeyMgmt8021X || accessPoint.RsnFlags & KeyMgmt8021X) {
                    accessPoint.security = Wpa2Enterprise;
                } else {
                    accessPoint.security = Wpa2Psk;
                }
            } else if ((accessPoint.RsnFlags & PairCcmp) && (capabilities & CcmpCipher)) {
                if (accessPoint.WpaFlags & KeyMgmt8021X || accessPoint.RsnFlags & KeyMgmt8021X) {
                    accessPoint.security = Wpa2Enterprise;
                } else {
                    accessPoint.security = Wpa2Psk;
                }
            } else if ((accessPoint.WpaFlags & KeyMgmtPsk) && (accessPoint.WpaFlags & PairTkip) && (capabilities & TkipCipher)) {
                if (accessPoint.WpaFlags & KeyMgmt8021X || accessPoint.RsnFlags & KeyMgmt8021X) {
                    accessPoint.security = WpaEnterprise;
                } else {
                    accessPoint.security = WpaPsk;
                }
            } else if ((accessPoint.WpaFlags & KeyMgmtPsk) && (accessPoint.WpaFlags & PairCcmp) && (capabilities & CcmpCipher)) {
                if (accessPoint.WpaFlags & KeyMgmt8021X || accessPoint.RsnFlags & KeyMgmt8021X) {
                    accessPoint.security = WpaEnterprise;
                } else {
                    accessPoint.security = WpaPsk;
                }
            } else if (accessPoint.WpaFlags == NoneSecurityFlags && accessPoint.RsnFlags == NoneSecurityFlags) {
                accessPoint.security = NoSecurity;
            } else if ((capabilities & Wep40Cipher) || (capabilities & Wep104Cipher)) {
                accessPoint.security = StaticWep;
            }

            QDBusInterface settings(deviceInterface->service(), "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());
            QList<QDBusObjectPath> connectionSettings = settings.property("Connections").value<QList<QDBusObjectPath>>();

            for (QDBusObjectPath settingsPath : connectionSettings) {
                QDBusMessage msg = QDBusMessage::createMethodCall(deviceInterface->service(), settingsPath.path(), "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
                QDBusPendingCall msgReply = QDBusConnection::systemBus().asyncCall(msg);

                QEventLoop* loop = new QEventLoop();
                QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(msgReply);
                connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), loop, SLOT(quit()));

                loop->exec();
                loop->deleteLater();
                watcher->deleteLater();

                if (msgReply.reply().arguments().count() != 0) {
                    QMap<QString, QVariantMap> settings;

                    QDBusArgument arg1 = msgReply.reply().arguments().first().value<QDBusArgument>();
                    arg1 >> settings;

                    for (QString key : settings.keys()) {
                        if (key == "802-11-wireless") {
                            QVariantMap wireless = settings.value("802-11-wireless");
                            if (wireless.value("ssid") == ssid) {
                                accessPoint.known = true;
                                break;
                            }
                        }
                    }
                }
            }

            accessPoints.append(accessPoint);
        }
    }

    this->accessPoints = accessPoints;
    this->dataChanged(this->index(0), this->index(this->rowCount()));
    reloadingNetworks = false;
}

QVariant AvailableNetworksList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    AccessPoint ap = accessPoints.at(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return ap.ssid;
        case Qt::DecorationRole: {
            if (ap.strength < 15) {
                return QIcon::fromTheme("network-wireless-connected-00");
            } else if (ap.strength < 35) {
                return QIcon::fromTheme("network-wireless-connected-25");
            } else if (ap.strength < 65) {
                return QIcon::fromTheme("network-wireless-connected-50");
            } else if (ap.strength < 85) {
                return QIcon::fromTheme("network-wireless-connected-75");
            } else {
                return QIcon::fromTheme("network-wireless-connected-100");
            }
        }
        case Qt::UserRole:
            return QVariant::fromValue(ap);
        case Qt::UserRole + 1: {
            if (ap.known) {
                return tr("Saved");
            } else {
                switch (ap.security) {
                    case Wpa2Psk:
                        return tr("Secured with WPA2-PSK");
                    case WpaPsk:
                        return tr("Secured with WPA-PSK");
                    case WpaEnterprise:
                        return tr("Secured with WPA Enterprise");
                    case Wpa2Enterprise:
                        return tr("Secured with WPA2 Enterprise");
                    case DynamicWep:
                        return tr("Secured with Dynamic WEP");
                    case StaticWep:
                        return tr("Secured with Static WEP");
                    case NoSecurity:
                        return tr("Not Secured");
                }
            }
        }
    }

    return QVariant();
}

AvailableNetworksListDelegate::AvailableNetworksListDelegate(QObject *parent) : QAbstractItemDelegate(parent) {

}


void AvailableNetworksListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);

    QRect iconRect;
    iconRect.setLeft(option.rect.left() + 6 * getDPIScaling());
    iconRect.setTop(option.rect.top() + 6 * getDPIScaling());
    iconRect.setBottom(iconRect.top() + 32 * getDPIScaling());
    iconRect.setRight(iconRect.left() + 32 * getDPIScaling());

    QRect textRect;
    textRect.setLeft(iconRect.right() + 6 * getDPIScaling());
    textRect.setTop(option.rect.top() + 6 * getDPIScaling());
    textRect.setBottom(option.rect.top() + option.fontMetrics.height() + 6 * getDPIScaling());
    textRect.setRight(option.rect.right());

    QRect descRect;
    descRect.setLeft(iconRect.right() + 6 * getDPIScaling());
    descRect.setTop(option.rect.top() + option.fontMetrics.height() + 8 * getDPIScaling());
    descRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + 6 * getDPIScaling());
    descRect.setRight(option.rect.right());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
        painter->drawText(textRect, index.data().toString());
        painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(descRect, index.data(Qt::UserRole + 1).toString());
    }
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
}

QSize AvailableNetworksListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int fontHeight = option.fontMetrics.height() * 2 + 14 * getDPIScaling();
    int iconHeight = 46 * getDPIScaling();

    return QSize(option.fontMetrics.width(index.data().toString()), qMax(fontHeight, iconHeight));
}

QDBusObjectPath AvailableNetworksList::devicePath() {
    return QDBusObjectPath(deviceInterface->path());
}
