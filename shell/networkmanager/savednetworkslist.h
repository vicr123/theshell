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
