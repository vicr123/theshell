#ifndef KDECONNECTDEVICESMODEL_H
#define KDECONNECTDEVICESMODEL_H

#include <QAbstractListModel>
#include <QDBusInterface>
#include <QStyledItemDelegate>
#include <QListView>
#include <QPainter>
#include <QTimer>

class KdeConnectDevicesModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit KdeConnectDevicesModel(QObject *parent = nullptr);

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    public slots:
        void updateData();

    private:
        QDBusInterface* daemon;
        QStringList devices;
};

class KdeConnectDevicesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        KdeConnectDevicesDelegate(QWidget *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


#endif // KDECONNECTDEVICESMODEL_H
