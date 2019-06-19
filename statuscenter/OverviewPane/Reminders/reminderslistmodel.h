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

#ifndef REMINDERSLISTMODEL_H
#define REMINDERSLISTMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <QPainter>
#include <QStyledItemDelegate>
#include <the-libs_global.h>
#include <QDateTime>
#include <debuginformationcollector.h>

class RemindersListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    RemindersListModel(QObject *parent = T_QOBJECT_ROOT);
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
