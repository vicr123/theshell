#include "notificationdbus.h"

extern QIcon getIconFromTheme(QString name, QColor textColor);

NotificationDBus::NotificationDBus(QObject *parent) : QObject(parent)
{
    new NotificationsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/freedesktop/Notifications", this);
    dbus.registerService("org.freedesktop.Notifications");
}

QStringList NotificationDBus::GetCapabilities() {
    return QStringList() << "body" << "body-markup" << "icon-static" << "persistence" << "actions";
}

uint NotificationDBus::Notify(QString app_name, uint replaces_id,
                              QString app_icon, QString summary,
                              QString body, QStringList actions,
                              QVariantMap hints, int expire_timeout) {
    if (replaces_id == 0) {
        replaces_id = nextId;
        nextId++;

        bool showDialog = true;
        if (dropdownPane != NULL) {
            if (dropdownPane->isQuietOn()) {
                if (hints.keys().contains("urgency")) {
                    if (hints.value("urgency").toChar() != 2) {
                        showDialog = false;
                    }
                } else {
                    showDialog = false;
                }
            }
        }

        NotificationDialog *d = new NotificationDialog(summary, body, actions, replaces_id, hints, expire_timeout);
        d->dbusParent = this;

        connect(d, SIGNAL(closing(int, int)), this, SLOT(sendCloseNotification(int, int)));

        d->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        if (showDialog) {
            d->show();
        }
        dialogs.append(d);
    } else {
        dialogs.at(replaces_id - 1)->setParams(summary, body);
        dialogs.at(replaces_id - 1)->show();
    }

    bool transient = false;
    if (hints.keys().contains("transient")) {
        if (hints.value("transient").toBool() == true) {
            transient = true;
        }
    }

    if (!transient) {

        QColor color = QApplication::palette("QLabel").color(QPalette::Window);
        QIcon icon = QIcon::fromTheme("dialog-warning");
        if (hints.keys().contains("category")) {
            QString category = hints.value("category").toString();
            if (category == "network.connected") {
                icon = QIcon::fromTheme("network-connect").pixmap(24, 24);
            } else if (category == "network.disconnected") {
                icon = QIcon::fromTheme("network-disconnect").pixmap(24, 24);
            } else if (category == "email.arrived") {
                icon = QIcon::fromTheme("mail-receive").pixmap(24, 24);
            } else if (category == "battery.charging") {
                icon = getIconFromTheme("battery-charging.svg", color).pixmap(24, 24);
            } else if (category == "battery.charged") {
                icon = getIconFromTheme("battery-charged.svg", color).pixmap(24, 24);
            } else if (category == "battery.discharging") {
                icon = getIconFromTheme("battery-not-charging.svg", color).pixmap(24, 24);
            } else if (category == "battery.low") {
                icon = getIconFromTheme("battery-low.svg", color).pixmap(24, 24);
            } else if (category == "battery.critical") {
                icon = getIconFromTheme("battery-critical.svg", color).pixmap(24, 24);
            }
        } else if (hints.keys().contains("urgency")) {
            QChar urgency = hints.value("urgency").toChar();
            if (urgency == 0) {
                icon = QIcon::fromTheme("dialog-information");
            } else if (urgency == 2) {
                icon = QIcon::fromTheme("dialog-error");
            }
        }
        emit newNotification(replaces_id, summary, body, icon);
    }
    return replaces_id;
}

QString NotificationDBus::GetServerInformation(QString &vendor, QString &version, QString &spec_version) {

    vendor = "theSuite";
    version = "1.0";
    spec_version = "1.2";
    return "theShell";
}

void NotificationDBus::CloseNotification(int id) {
    NotificationDialog *d = dialogs.at(id - 1);
    d->close(3);
}

void NotificationDBus::CloseNotificationUserInitiated(int id) {
    NotificationDialog *d = dialogs.at(id - 1);
    d->close(2);
}

void NotificationDBus::sendCloseNotification(int id, int reason) {
    emit NotificationClosed(id, reason);

    if (reason == 2 || reason == 3) {
        emit removeNotification(id);
    }
}

void NotificationDBus::invokeAction(uint id, QString key) {
    emit ActionInvoked(id, key);
}

void NotificationDBus::setDropdownPane(InfoPaneDropdown *pane) {
    this->dropdownPane = pane;
}
