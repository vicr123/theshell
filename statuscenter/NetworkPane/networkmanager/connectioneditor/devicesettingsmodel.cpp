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
#include "devicesettingsmodel.h"

#include <the-libs_global.h>
#include <QPainter>
#include <functional>

#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include "connectioneditor.h"

struct DeviceSettingsModelItem {
    enum ItemAction {
        Widget,
        NewConnection
    };

    DeviceSettingsModelItem(NetworkManager::Connection::Ptr connection, NetworkManager::Device::Ptr device) {
        this->connection = connection;
        this->getWidgetFunction = [=] {
            return new ConnectionEditor(device, connection);
        };
        action = Widget;
    }

    DeviceSettingsModelItem(QString text, QIcon icon, std::function<QWidget*()> getWidget = [] {return nullptr;}) {
        this->text = text;
        this->icon = icon;
        this->getWidgetFunction = getWidget;
        action = Widget;
    }

    DeviceSettingsModelItem(QString text, QIcon icon, ItemAction action) {
        this->text = text;
        this->icon = icon;
        this->action = action;
    }

    DeviceSettingsModelItem() {
        isSeperator = true;
        this->getWidgetFunction = [] {return nullptr;};
        action = Widget;
    }

    ItemAction action;

    QWidget* cachedWidget = nullptr;
    QWidget* getWidget() {
        if (cachedWidget == nullptr) {
            cachedWidget = getWidgetFunction();
        }
        return cachedWidget;
    }

    NetworkManager::Connection::Ptr connection;

    bool isSeperator = false;

    QString text;
    QIcon icon;
    std::function<QWidget*()> getWidgetFunction;
};

struct DeviceSettingsModelPrivate {
    NetworkManager::Device::Ptr device;
    QList<QSharedPointer<DeviceSettingsModelItem>> items;
};

DeviceSettingsModel::DeviceSettingsModel(NetworkManager::Device::Ptr device, QObject *parent)
    : QAbstractListModel(parent)
{
    d = new DeviceSettingsModelPrivate();
    d->device = device;

    connect(d->device.data(), &NetworkManager::Device::availableConnectionAppeared, this, [=](QString connection) {
        for (QSharedPointer<DeviceSettingsModelItem> item : d->items) {
            if (item->connection->path() == connection) {
                //Don't do anything
                return;
            }
        }

        //Add this connection to the list
        NetworkManager::Connection::Ptr newConnection = NetworkManager::Connection::Ptr(new NetworkManager::Connection(connection));
        addConnection(newConnection);
    });

    for (NetworkManager::Connection::Ptr connection : d->device->availableConnections()) {
        addConnection(connection);
    }

    d->items.append(QSharedPointer<DeviceSettingsModelItem>(new DeviceSettingsModelItem(tr("New Connection"), QIcon::fromTheme("list-add"), DeviceSettingsModelItem::NewConnection)));
    d->items.append(QSharedPointer<DeviceSettingsModelItem>(new DeviceSettingsModelItem()));
    switch (device->type()) {
        case NetworkManager::Device::Modem: {
            //Check if this is actually a cellular modem
            d->items.append(QSharedPointer<DeviceSettingsModelItem>(new DeviceSettingsModelItem(tr("SIM PIN"), QIcon::fromTheme("sim-card"))));
            break;
        }
        default: ; //Do nothing here
    }
}

DeviceSettingsModel::~DeviceSettingsModel() {
    delete d;
}

int DeviceSettingsModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    // FIXME: Implement me!
    return d->items.count();
}

QVariant DeviceSettingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QSharedPointer<DeviceSettingsModelItem> item = d->items.at(index.row());
    if (item->connection.isNull()) {
        switch (role) {
            case Qt::DisplayRole:
                return item->text;
            case Qt::DecorationRole:
                return item->icon;
            case Qt::UserRole + 1:
                return true;
            case Qt::UserRole + 2:
                return item->isSeperator;
        }
    } else {
        NetworkManager::Connection::Ptr connection = item->connection;
        NetworkManager::ConnectionSettings::Ptr settings = connection->settings();
        switch (role) {
            case Qt::DisplayRole:
                return connection->name();
            case Qt::DecorationRole:
                switch (settings->connectionType()) {
                    case NetworkManager::ConnectionSettings::Bluetooth:
                        return QIcon::fromTheme("bluetooth");
                    case NetworkManager::ConnectionSettings::Wireless:
                        return QIcon::fromTheme("network-wireless");
                    case NetworkManager::ConnectionSettings::Gsm:
                        return QIcon::fromTheme("network-cellular");
                    default:
                        return QIcon::fromTheme("network-wired");
                }
            case Qt::UserRole + 1:
                return false;
        }
    }
    return QVariant();
}

void DeviceSettingsModel::activateItem(QModelIndex index) {

    QSharedPointer<DeviceSettingsModelItem> item = d->items.at(index.row());
    if (item->connection.isNull()) {
        switch (item->action) {
            case DeviceSettingsModelItem::NewConnection: {
                //Create a new connection
                QScopedPointer<NetworkManager::ConnectionSettings> settings;
                switch (d->device->type()) {
                    case NetworkManager::Device::Modem:
                        settings.reset(new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Gsm));
                        break;
                }

                settings->setId(tr("New Connection"));
                settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());

                QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(NetworkManager::addConnectionUnsaved(settings->toMap()));
                connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
                    QDBusMessage reply = watcher->reply();
                    QString error = reply.errorName();
                    QString errorMessage = reply.errorMessage();

                    //Add this connection to the list
                    NetworkManager::Connection::Ptr newConnection = NetworkManager::Connection::Ptr(new NetworkManager::Connection(watcher->reply().arguments().first().value<QDBusObjectPath>().path()));
                    addConnection(newConnection, index.row());

                    //Edit this item
                    activateSecondary(index);
                });
                break;
            }
            case DeviceSettingsModelItem::Widget: {
                QWidget* w = configurationWidgetForIndex(index);
                if (w != nullptr) emit showWidget(w);
            }
        }
    } else {
        //Activate this connection on the device
        NetworkManager::Connection::Ptr connection = item->connection;
        NetworkManager::activateConnection(connection->path(), d->device->uni(), "/");

        emit closeDialog();
    }
}

void DeviceSettingsModel::activateSecondary(QModelIndex index) {
    QSharedPointer<DeviceSettingsModelItem> item = d->items.at(index.row());
    if (item->connection.isNull()) {
        //There's no secondary button here so just act as if the normal item was activated
        activateItem(index);
    } else {
        //Open the configuration widget
        QWidget* w = configurationWidgetForIndex(index);
        if (w != nullptr) emit showWidget(w);
    }
}

void DeviceSettingsModel::addConnection(NetworkManager::Connection::Ptr connection, int index) {
    connect(connection.data(), &NetworkManager::Connection::updated, this, [=] {
        emit dataChanged(this->index(0), this->index(rowCount()));
    });
    connect(connection.data(), &NetworkManager::Connection::removed, this, [=](QString connection) {
        for (QSharedPointer<DeviceSettingsModelItem> item : d->items) {
            if (item->connection->path() == connection) {
                //Remove this one
                d->items.removeOne(item);
                emit dataChanged(this->index(0), this->index(rowCount()));
                return;
            }
        }
    });

    QSharedPointer<DeviceSettingsModelItem> newItem(new DeviceSettingsModelItem(connection, d->device));
    if (index == -1) {
        d->items.append(newItem);
    } else {
        d->items.insert(index, newItem);
    }
    emit dataChanged(this->index(0), this->index(rowCount()));
}

QWidget* DeviceSettingsModel::configurationWidgetForIndex(QModelIndex index) {
    return d->items.at(index.row())->getWidget();
}

DeviceSettingsDelegate::DeviceSettingsDelegate(QObject* parent) : QStyledItemDelegate(parent) {

}

void DeviceSettingsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(Qt::UserRole + 1).toBool()) {
        //Special item
        if (index.data(Qt::UserRole + 2).toBool()) {
            //Seperator
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::WindowText));
            painter->drawRect(option.rect);
        } else {
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else {
        //Normal item
        QStyleOptionViewItem optionCopy = option;
        optionCopy.rect.setWidth(optionCopy.rect.width() - optionCopy.rect.height());
        QStyledItemDelegate::paint(painter, optionCopy, index);

        QRect editIconRect;
        editIconRect.setSize(QSize(optionCopy.rect.height(), optionCopy.rect.height()));
        editIconRect.moveTopLeft(optionCopy.rect.topRight());

        QRect editIconIconRect;
        editIconIconRect.setSize(option.decorationSize);
        editIconIconRect.moveCenter(editIconRect.center());
        painter->drawPixmap(editIconIconRect, QIcon::fromTheme("configure").pixmap(editIconIconRect.size()));
    }
}

QSize DeviceSettingsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize sz = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(Qt::UserRole + 1).toBool()) {
        //Check if this is a seperator
        if (index.data(Qt::UserRole + 2).toBool()) sz.setHeight(1);
    }
    return sz;
}
