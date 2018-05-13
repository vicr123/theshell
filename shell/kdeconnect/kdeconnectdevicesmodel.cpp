#include "kdeconnectdevicesmodel.h"

extern float getDPIScaling();

KdeConnectDevicesModel::KdeConnectDevicesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    daemon = new QDBusInterface("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon");

    QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon", "deviceAdded", this, SLOT(updateData()));
    QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon", "deviceRemoved", this, SLOT(updateData()));
    QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon", "deviceRemoved", this, SLOT(deviceVisibilityChanged()));

    updateData();
}

void KdeConnectDevicesModel::updateData() {
    for (QString device : devices) {
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "reachableChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "pluginsChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "trustedChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "nameChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "hasPairingRequestsChanged", this, SLOT(updateData()));
    }

    if (daemon->isValid()) {
        devices = daemon->call("devices", false, false).arguments().first().toStringList();
    } else {
        devices.clear();
    }

    for (QString device : devices) {
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "reachableChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "pluginsChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "trustedChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "nameChanged", this, SLOT(updateData()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device", "hasPairingRequestsChanged", this, SLOT(updateData()));
    }

    this->dataChanged(this->index(0), this->index(this->rowCount()));
}

int KdeConnectDevicesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return devices.count();
}

int KdeConnectDevicesModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 1;
}

QVariant KdeConnectDevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QDBusInterface device("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + devices.value(index.row()), "org.kde.kdeconnect.device");
    if (role == Qt::DisplayRole) {
        return device.property("name").toString();
    } else if (role == Qt::DecorationRole) {
        QString type = device.property("type").toString();
        if (type == "tablet") {
            return QIcon::fromTheme("tablet");
        } else if (type == "desktop") {
            return QIcon::fromTheme("computer");
        } else {
            return QIcon::fromTheme("phone");
        }
    } else if (role == Qt::UserRole) {
        QStringList s;
        QString type = device.property("type").toString();
        if (type == "tablet") {
            s.append("Tablet");
        } else if (type == "desktop") {
            s.append("Computer");
        } else if (type == "smartphone") {
            s.append("Phone");
        } else {
            s.append("Device");
        }

        if (!device.property("isTrusted").toBool()) {
            s.append("Ready to pair");
        }
        return s.join(" · ");
    } else if (role == Qt::UserRole + 1) {
        return devices.value(index.row());
    }
    return QVariant();
}

KdeConnectDevicesDelegate::KdeConnectDevicesDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void KdeConnectDevicesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);

    QRect iconRect;
    if (((QListView*) option.widget)->viewMode() == QListView::IconMode) {
        iconRect.setLeft(option.rect.left() + 38 * getDPIScaling());
        iconRect.setTop(option.rect.top() + 6 * getDPIScaling());
        iconRect.setHeight(32 * getDPIScaling());
        iconRect.setWidth(32 * getDPIScaling());

        QRect textRect;
        textRect.setLeft(option.rect.left() + 6 * getDPIScaling());
        textRect.setTop(iconRect.bottom() + 6 * getDPIScaling());
        textRect.setBottom(option.rect.bottom());
        textRect.setRight(option.rect.right());

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, index.data().toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
        }
    } else {
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

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, index.data().toString());
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
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
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
        }
    }
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QIcon>().pixmap(iconRect.size()));

    QDBusInterface device("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + index.data(Qt::UserRole + 1).toString(), "org.kde.kdeconnect.device");
    bool reachable = device.property("isReachable").toBool();
    bool trusted = device.property("isTrusted").toBool();

    if (reachable && trusted) {
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect.left(), option.rect.top(), 6 * getDPIScaling(), option.rect.height());
    }
}

QSize KdeConnectDevicesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (((QListView*) option.widget)->viewMode() == QListView::IconMode) {
        return QSize(128 * getDPIScaling(), 128 * getDPIScaling());
    } else {
        int fontHeight = option.fontMetrics.height() * 2 + 14 * getDPIScaling();
        int iconHeight = 46 * getDPIScaling();

        return QSize(option.fontMetrics.width(index.data().toString()), qMax(fontHeight, iconHeight));
    }
}
