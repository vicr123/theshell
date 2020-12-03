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

#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>
#include <QFuture>
#include <QStyleOptionViewItem>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QSettings>
#include <QFontInfo>
#include <QMimeType>
#include <QMimeDatabase>

#undef True
#undef False

#include <QtConcurrent>
#include <QPainter>
#include <QListView>
#include "bthandsfree.h"
#include "nativeeventfilter.h"

struct AppsListModelPrivate;
class AppsListModel : public QAbstractListModel {
        Q_OBJECT

    public:
        AppsListModel(QObject* parent = 0);
        ~AppsListModel();

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void updateData();
        bool launchApp(QModelIndex index);
        void search(QString query);

        int pinnedAppsCount();

        QString currentQuery = "";

    public slots:
        void loadData();

    signals:

    private:
        AppsListModelPrivate* d;
};

class AppsDelegate : public QStyledItemDelegate {
        Q_OBJECT

    public:
        AppsDelegate(QWidget* parent = 0, bool drawArrows = true);
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
        bool drawArrows;
};

#endif // APPSLISTMODEL_H
