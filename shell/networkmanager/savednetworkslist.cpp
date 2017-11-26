#include "savednetworkslist.h"

SavedNetworksList::SavedNetworksList(QObject *parent)
    : QAbstractListModel(parent)
{
    savedInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());

    QDBusConnection::systemBus().connect(savedInterface->service(), savedInterface->path(), savedInterface->interface(), "NewConnection", this, SLOT(loadNetworks()));
    QDBusConnection::systemBus().connect(savedInterface->service(), savedInterface->path(), savedInterface->interface(), "ConnectionRemoved", this, SLOT(loadNetworks()));

    loadNetworks();
}

int SavedNetworksList::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return savedNetworks.count();
}

QVariant SavedNetworksList::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    Setting s = savedNetworks.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return s.name;
        case Qt::DecorationRole:
            switch (s.NetworkType) {
                case Setting::Wireless:
                    return QIcon::fromTheme("network-wireless");
                case Setting::Bluetooth:
                    return QIcon::fromTheme("network-bluetooth");
                case Setting::Mobile:
                    return QIcon::fromTheme("network-cellular");
                case Setting::VPN:
                    return QIcon::fromTheme("network-vpn");
                case Setting::Wired:
                default:
                    return QIcon::fromTheme("network-wired");
            }
        case Qt::UserRole:
            return QVariant::fromValue(s);
        case Qt::UserRole + 1:
            return tr("Saved");
    }
    return QVariant();
}

void SavedNetworksList::loadNetworks() {
    savedNetworks.clear();
    QDBusMessage m = savedInterface->call(QDBus::Block, "ListConnections");
    QDBusArgument knownNetworks = m.arguments().first().value<QDBusArgument>();
    knownNetworks.beginArray();
    while (!knownNetworks.atEnd()) {
        QDBusObjectPath o;
        knownNetworks >> o;

        QDBusMessage settingsMessage = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", o.path(), "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
        QDBusArgument settingsArg = QDBusConnection::systemBus().call(settingsMessage).arguments().first().value<QDBusArgument>();
        QMap<QString, QVariantMap> settings;
        settingsArg >> settings;

        Setting s;
        QVariantMap connectionSettings = settings.value("connection");
        s.path = o;
        s.name = connectionSettings.value("id").toString();

        if (connectionSettings.value("type") == "802-3-ethernet") {
            s.NetworkType = Setting::Wired;
        } else if (connectionSettings.value("type") == "802-11-wireless") {
            s.NetworkType = Setting::Wireless;
        } else if (connectionSettings.value("type") == "bluetooth") {
            s.NetworkType = Setting::Bluetooth;
        } else if (connectionSettings.value("type") == "vpn") {
            s.NetworkType = Setting::VPN;
        } else if (connectionSettings.value("type") == "gsm" || connectionSettings.value("type") == "cdma") {
            s.NetworkType = Setting::Mobile;
        } else {
            s.NetworkType = Setting::Unknown;
        }

        savedNetworks.append(s);
    }
    knownNetworks.endArray();

    this->dataChanged(this->index(0), this->index(this->rowCount()));
}

void Setting::del() {
    QDBusMessage deleteMessage = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", this->path.path(), "org.freedesktop.NetworkManager.Settings.Connection", "Delete");
    QDBusConnection::systemBus().call(deleteMessage, QDBus::NoBlock);
}
