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

#include "transferslistmodel.h"

extern float getDPIScaling();

TransfersListModel::TransfersListModel(BluezQt::ObexManager* mgr, QObject *parent)
    : QAbstractListModel(parent)
{
    this->mgr = mgr;
}

int TransfersListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return transfers.count();
}

QVariant TransfersListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    BluezQt::ObexTransferPtr transfer = transfers.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            if (transfer.data()->fileName() == "") {
                return transfer.data()->name();
            } else {
                return QFileInfo(transfer.data()->fileName()).baseName();
            }
        case Qt::DecorationRole:
            return QIcon::fromTheme("document-new");
        case Qt::UserRole:
            return QVariant::fromValue(transfer);
    }

    return QVariant();
}

void TransfersListModel::pushTransfer(BluezQt::ObexTransferPtr transfer, bool incoming) {
    transfers.prepend(transfer);
    connect(transfer.data(), &BluezQt::ObexTransfer::transferredChanged, [=] {
        emit dataChanged(index(transfers.indexOf(transfer)), index(transfers.indexOf(transfer)));
    });
    connect(transfer.data(), &BluezQt::ObexTransfer::statusChanged, [=](BluezQt::ObexTransfer::Status status) {
        emit dataChanged(index(transfers.indexOf(transfer)), index(transfers.indexOf(transfer)));

        if (status == BluezQt::ObexTransfer::Complete) {
            tNotification* n = new tNotification();
            n->setAppName("Bluetooth");
            n->setAppIcon("preferences-system-bluetooth");
            n->setSummary(tr("Transfer Complete"));
            if (incoming) {
                n->setText(tr("<b>%1</b> has been successfully received and placed inside Downloads > Bluetooth.").arg(transfer.data()->name()));
            } else {
                n->setText(tr("<b>%1</b> has been sent successfully.").arg(transfer.data()->name()));
            }
            n->post();
        } else if (status == BluezQt::ObexTransfer::Error) {
            tNotification* n = new tNotification();
            n->setAppName("Bluetooth");
            n->setAppIcon("preferences-system-bluetooth");
            n->setSummary(tr("Transfer Failure"));
            if (incoming) {
                n->setText(tr("There was a problem while receiving <b>%1</b>.").arg(transfer.data()->name()));
            } else {
                n->setText(tr("There was a problem while sending <b>%1</b>.").arg(transfer.data()->name()));
            }
            n->post();
        }
    });
    emit dataChanged(index(0), index(rowCount()));
}

TransfersDelegate::TransfersDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void TransfersDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    BluezQt::ObexTransferPtr transfer = index.data(Qt::UserRole).value<BluezQt::ObexTransferPtr>();
    painter->setFont(option.font);
    painter->setLayoutDirection(option.direction);

    QRect iconRect;
    iconRect.setLeft(option.rect.left() + 12 * getDPIScaling());
    iconRect.setTop(option.rect.top() + 6 * getDPIScaling());
    iconRect.setBottom(iconRect.top() + 32 * getDPIScaling());
    iconRect.setRight(iconRect.left() + 32 * getDPIScaling());

    QRect textRect;
    textRect.setLeft(iconRect.right() + 6 * getDPIScaling());
    textRect.setTop(option.rect.top() + 6 * getDPIScaling());
    textRect.setBottom(option.rect.top() + option.fontMetrics.height() + 6 * getDPIScaling());
    textRect.setRight(option.rect.right());

    QRect descRect;
    descRect.setLeft(iconRect.right() + 6 * getDPIScaling());
    descRect.setTop(option.rect.top() + option.fontMetrics.height() + 8 * getDPIScaling());
    descRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + 6 * getDPIScaling());
    descRect.setRight(option.rect.right() - 6 * getDPIScaling());

    if (option.direction == Qt::RightToLeft) {
        iconRect.moveRight(option.rect.right() - 6 * getDPIScaling());
        textRect.moveRight(iconRect.left() - 6 * getDPIScaling());
        descRect.moveRight(iconRect.left() - 6 * getDPIScaling());
    }

    QString descText;
    switch (transfer.data()->status()) {
        case BluezQt::ObexTransfer::Error: {
            descText = tr("Failed");
            break;
        }
        case BluezQt::ObexTransfer::Complete: {
            descText = tr("Done");
            break;
        }
        case BluezQt::ObexTransfer::Queued: {
            descText = tr("Queued");
            break;
        }
        case BluezQt::ObexTransfer::Suspended: {
            descText = tr("Paused");
            break;
        }
        case BluezQt::ObexTransfer::Unknown: {
            descText = tr("Unknown");
            break;
        }
        case BluezQt::ObexTransfer::Active: {
            //do nothing
            break;
        }
    }


    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
        painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        painter->drawText(descRect, Qt::AlignLeading, descText);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(descRect, Qt::AlignLeading, descText);
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(descRect, Qt::AlignLeading, descText);
    }

    if (transfer.data()->status() == BluezQt::ObexTransfer::Active) {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->setBrush(option.palette.color(QPalette::Window));
        painter->drawRect(descRect);
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));

        int width = ((float) transfer.data()->transferred() / (float) transfer.data()->size()) * descRect.width() - 1;
        if (width < 0) width = 0;
        painter->drawRect(descRect.x() + 1, descRect.y() + 1, width - 1, descRect.height() - 1);
    }

    if (transfer.data()->status() == BluezQt::ObexTransfer::Error) {
        painter->setBrush(QColor(100, 0, 0));
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect.left(), option.rect.top(), 6 * getDPIScaling(), option.rect.height());
    }

    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
}

QSize TransfersDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int fontHeight = option.fontMetrics.height() * 2 + 14 * getDPIScaling();
    int iconHeight = 46 * getDPIScaling();

    return QSize(option.fontMetrics.width(index.data().toString()), qMax(fontHeight, iconHeight));
}
