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
#include "applicationnotificationmodel.h"

#include <QIcon>
#include <QPainter>

struct ApplicationNotificationModelPrivate {
    QList<ApplicationInformation> appInformation;
};

ApplicationNotificationModel::ApplicationNotificationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new ApplicationNotificationModelPrivate;

    loadData();
}

ApplicationNotificationModel::~ApplicationNotificationModel() {
    delete d;
}

int ApplicationNotificationModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return d->appInformation.count() + 1;
}

QVariant ApplicationNotificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();
    if (row == 0) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("General");
            case Qt::DecorationRole:
                return QIcon::fromTheme("preferences-system-notifications");
        }
    } else {
        row -= 1;
        ApplicationInformation info = d->appInformation.at(row);
        switch (role) {
            case Qt::DisplayRole:
                return info.name;
            case Qt::DecorationRole:
                return info.icon;
            case Qt::UserRole:
                return QVariant::fromValue(info);
        }
    }
    return QVariant();
}

void ApplicationNotificationModel::loadData() {
    d->appInformation.clear();
    QStringList allDesktopApps = NotificationsPermissionEngine::knownDesktopFiles();
    for (QString app : allDesktopApps) {
        NotificationsPermissionEngine permissions("", app);

        ApplicationInformation info;
        info.desktopEntry = app;
        info.name = permissions.appName();
        info.icon = permissions.appIcon();
        d->appInformation.append(info);
    }

    QStringList allApps = NotificationsPermissionEngine::knownApps();
    for (QString app : allApps) {
        NotificationsPermissionEngine permissions(app);

        ApplicationInformation info;
        info.name = permissions.appName();
        info.icon = info.permissionsEngine()->appIcon();
        d->appInformation.append(info);
    }

    emit dataChanged(index(0), index(rowCount()));
}

ApplicationNotificationModelDelegate::ApplicationNotificationModelDelegate(QObject* parent) : QStyledItemDelegate(parent)
{

}

void ApplicationNotificationModelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
    if (index.row() == 0) {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    }
}

QSize ApplicationNotificationModelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.row() == 0) {
        QSize sz = QStyledItemDelegate::sizeHint(option, index);
        sz.rheight() += 1;
        return sz;
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}
