#ifndef TRANSFERSLISTMODEL_H
#define TRANSFERSLISTMODEL_H

#include <QAbstractListModel>
#include <BluezQt/ObexManager>
#include <BluezQt/ObexSession>
#include <BluezQt/ObexFileTransfer>
#include <BluezQt/ObexObjectPush>
#include <BluezQt/ObexTransfer>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileInfo>

class TransfersListModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit TransfersListModel(BluezQt::ObexManager* mgr, QObject *parent = nullptr);

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        void pushTransfer(BluezQt::ObexTransferPtr transfer);

    private:
        BluezQt::ObexManager* mgr;
        QList<BluezQt::ObexTransferPtr> transfers;
};

class TransfersDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        TransfersDelegate(QWidget *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TRANSFERSLISTMODEL_H
