#include "deviceslistmodel.h"

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}

DevicesListModel::DevicesListModel(BluezQt::Manager* mgr, ShowDevices flags, QObject *parent)
    : QAbstractListModel(parent)
{
    this->mgr = mgr;
    this->flags = flags;

    connect(mgr, &BluezQt::Manager::deviceAdded, [=](BluezQt::DevicePtr device) {
        this->addDevice(device);
    });
    connect(mgr, &BluezQt::Manager::deviceRemoved, [=](BluezQt::DevicePtr device) {
        showingDevices.removeOne(device);
        emit dataChanged(index(0), index(rowCount()));
    });
    connect(mgr, &BluezQt::Manager::deviceChanged, [=](BluezQt::DevicePtr device) {
        if (showingDevices.contains(device)) {
            int row = showingDevices.indexOf(device);
            emit dataChanged(index(row), index(row));
        } else {
            addDevice(device);
        }
    });

    connect(mgr, &BluezQt::Manager::operationalChanged, [=](bool operational) {
        if (operational) {
            showingDevices.clear();
            for (BluezQt::DevicePtr device : mgr->devices()) {
                addDevice(device);
            }
        }
    });
}

int DevicesListModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid()) {
        return 0;
    }

    return showingDevices.count();
}

void DevicesListModel::addDevice(BluezQt::DevicePtr device) {
    if ((device.data()->isPaired() && flags.testFlag(Paired)) ||
            (!device.data()->isPaired() && flags.testFlag(Unpaired)) ||
            (device.data()->isConnected() && flags.testFlag(Connected)) ||
            (!device.data()->isConnected() && !device.data()->isPaired() && flags.testFlag(Unknown))) {
        showingDevices.append(device);

        emit dataChanged(index(0), index(rowCount()));
    }
}

QVariant DevicesListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // FIXME: Implement me!
    BluezQt::DevicePtr device = showingDevices.at(index.row());
    switch (role) {
        case Qt::DisplayRole:
            return device.data()->name();
        case Qt::UserRole:
            switch (device.data()->type()) {
                case BluezQt::Device::Phone:
                    return tr("Mobile Phone");
                case BluezQt::Device::Computer:
                    return tr("Computer");
                case BluezQt::Device::AudioVideo:
                    return tr("AV");
                case BluezQt::Device::Headphones:
                case BluezQt::Device::Headset:
                    return tr("Speaker");
                case BluezQt::Device::Camera:
                    return tr("Camera");
                case BluezQt::Device::Health:
                    return tr("Health");
                case BluezQt::Device::Imaging:
                    return tr("Imaging");
                case BluezQt::Device::Keyboard:
                    return tr("Keyboard");
                case BluezQt::Device::Mouse:
                    return tr("Mouse");
                case BluezQt::Device::Joypad:
                    return tr("Joypad");
                case BluezQt::Device::Peripheral:
                    return tr("Peripheral");
                case BluezQt::Device::Printer:
                    return tr("Printer");
                case BluezQt::Device::Network:
                    return tr("Network");
                case BluezQt::Device::Modem:
                    return tr("Modem");
                default:
                    return tr("Unknown");
            }
        case Qt::DecorationRole:
            return QIcon::fromTheme(device.data()->icon());
        case Qt::UserRole + 1:
            return QVariant::fromValue(device);
    }
    return QVariant();
}

DevicesDelegate::DevicesDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void DevicesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
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
    descRect.setRight(option.rect.right());

    if (option.direction == Qt::RightToLeft) {
        iconRect.moveRight(option.rect.right() - 6 * getDPIScaling());
        textRect.moveRight(iconRect.left() - 6 * getDPIScaling());
        descRect.moveRight(iconRect.left() - 6 * getDPIScaling());
    }

    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
        painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
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
        painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
    }
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));

    if (index.data(Qt::UserRole + 1).value<BluezQt::DevicePtr>().data()->isConnected()) {
        painter->setBrush(QColor(0, 100, 0));
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect.left(), option.rect.top(), 6 * getDPIScaling(), option.rect.height());
    }
}

QSize DevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int fontHeight = option.fontMetrics.height() * 2 + 14 * getDPIScaling();
    int iconHeight = 46 * getDPIScaling();

    return QSize(option.fontMetrics.width(index.data().toString()), qMax(fontHeight, iconHeight));
}
