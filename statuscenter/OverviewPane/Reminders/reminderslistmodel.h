#ifndef REMINDERSLISTMODEL_H
#define REMINDERSLISTMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <QPainter>
#include <QStyledItemDelegate>
#include <the-libs_global.h>
#include <QDateTime>

class RemindersListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    RemindersListModel(QObject *parent = nullptr);
    ~RemindersListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateData();

private:
    QSettings* RemindersData;
};

class RemindersDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RemindersDelegate(QWidget *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // REMINDERSLISTMODEL_H
