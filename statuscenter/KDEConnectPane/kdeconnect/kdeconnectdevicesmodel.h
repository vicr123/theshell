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

#ifndef KDECONNECTDEVICESMODEL_H
#define KDECONNECTDEVICESMODEL_H

#include <QAbstractListModel>
#include <QDBusInterface>
#include <QStyledItemDelegate>
#include <QListView>
#include <QPainter>
#include <QTimer>
#include <debuginformationcollector.h>

class KdeConnectDevicesModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit KdeConnectDevicesModel(QObject *parent = T_QOBJECT_ROOT);

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
