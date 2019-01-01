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

#include "reminderslistmodel.h"

RemindersListModel::RemindersListModel(QObject *parent) : QAbstractListModel(parent) {
    RemindersData = new QSettings("theSuite/theShell.reminders");
    RemindersData->beginGroup("reminders");
}

RemindersListModel::~RemindersListModel() {
    RemindersData->endGroup();
    RemindersData->deleteLater();
}

int RemindersListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    int count = RemindersData->beginReadArray("reminders");
    RemindersData->endArray();
    return count;
}

QVariant RemindersListModel::data(const QModelIndex &index, int role) const {
    QVariant returnValue;

    RemindersData->beginReadArray("reminders");
    RemindersData->setArrayIndex(index.row());
    if (role == Qt::DisplayRole) {
        returnValue = RemindersData->value("title");
    } else if (role == Qt::UserRole) {
        QDateTime activation = RemindersData->value("date").toDateTime();
        if (activation.daysTo(QDateTime::currentDateTime()) == 0) {
            returnValue = activation.toString("hh:mm");
        } else if (activation.daysTo(QDateTime::currentDateTime()) < 7) {
            returnValue = activation.toString("dddd");
        } else {
            returnValue = activation.toString("ddd, dd MMM yyyy");
        }
    }

    RemindersData->endArray();
    return returnValue;
}

void RemindersListModel::updateData() {
    emit dataChanged(index(0), index(rowCount()));
}

RemindersDelegate::RemindersDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void RemindersDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);

    QRect textRect;
    textRect.setLeft(6 * theLibsGlobal::getDPIScaling());
    textRect.setTop(option.rect.top() + 6 * theLibsGlobal::getDPIScaling());
    textRect.setBottom(option.rect.top() + option.fontMetrics.height() + 6 * theLibsGlobal::getDPIScaling());
    textRect.setRight(option.rect.right());

    QRect dateRect;
    dateRect.setLeft(6 * theLibsGlobal::getDPIScaling());
    dateRect.setTop(option.rect.top() + option.fontMetrics.height() + 8 * theLibsGlobal::getDPIScaling());
    dateRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + 6 * theLibsGlobal::getDPIScaling());
    dateRect.setRight(option.rect.right());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
        painter->drawText(textRect, index.data().toString());
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    }
}

QSize RemindersDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.fontMetrics.width(index.data().toString()), option.fontMetrics.height() * 2 + 14 * theLibsGlobal::getDPIScaling());
}
