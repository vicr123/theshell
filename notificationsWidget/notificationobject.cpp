#include "notificationobject.h"

int NotificationObject::currentId = 0;

NotificationObject::NotificationObject(QString app_name, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout, QObject *parent) : QObject(parent) {
    currentId++;
    this->id = currentId;

    dialog = new NotificationPopup(currentId);
    connect(dialog, SIGNAL(actionClicked(QString)), this, SIGNAL(actionClicked(QString)));
    connect(dialog, &NotificationPopup::notificationClosed, [=](uint reason) {
        emit closed((NotificationCloseReason) reason);
    });

    setParameters(app_name, app_icon, summary, body, actions, hints, expire_timeout);
}

void NotificationObject::setParameters(QString &app_name, QString &app_icon, QString &summary, QString &body, QStringList &actions, QVariantMap &hints, int expire_timeout) {
    this->appName = app_name;
    this->appIcon = app_icon;
    this->summary = summary;
    this->body = body;
    this->actions = actions;
    this->hints = hints;
    this->timeout = expire_timeout;

    appIc = QIcon::fromTheme("generic-app");
    if (appIcon != "" && QIcon::hasThemeIcon(appIcon)) {
        appIc = QIcon::fromTheme(appIcon);
    } else if (QIcon::hasThemeIcon(appName.toLower().replace(" ", "-"))) {
        appIc = QIcon::fromTheme(appName.toLower().replace(" ", "-"));
    } else if (QIcon::hasThemeIcon(appName.toLower().replace(" ", ""))) {
        appIc = QIcon::fromTheme(appName.toLower().replace(" ", ""));
    } else {
        QString filename = hints.value("desktop-entry", "").toString() + ".desktop";

        QDir appFolder("/usr/share/applications/");
        QDirIterator iterator(appFolder, QDirIterator::Subdirectories);

        while (iterator.hasNext()) {
            iterator.next();
            QFileInfo info = iterator.fileInfo();
            if (info.fileName() == filename || info.baseName().toLower() == appName.toLower()) {
                QFile file(info.filePath());
                file.open(QFile::ReadOnly);
                QString appinfo(file.readAll());

                QStringList desktopLines;
                QString currentDesktopLine;
                for (QString desktopLine : appinfo.split("\n")) {
                    if (desktopLine.startsWith("[") && currentDesktopLine != "") {
                        desktopLines.append(currentDesktopLine);
                        currentDesktopLine = "";
                    }
                    currentDesktopLine.append(desktopLine + "\n");
                }
                desktopLines.append(currentDesktopLine);

                for (QString desktopPart : desktopLines) {
                    for (QString line : desktopPart.split("\n")) {
                        if (line.startsWith("icon=", Qt::CaseInsensitive)) {
                            QString iconname = line.split("=")[1];
                            if (QFile(iconname).exists()) {
                                appIc = QIcon(iconname);
                            } else {
                                appIc = QIcon::fromTheme(iconname, QIcon::fromTheme("application-x-executable"));
                            }
                        }
                    }
                }
            }
        }
    }

    bigIc = QIcon();
    if (hints.value("x-thesuite-timercomplete", false).toBool()) {
        bigIc = QIcon::fromTheme("chronometer");
    } else {
        if (hints.keys().contains("category")) {
            QString category = hints.value("category").toString();
            if (category == "network.connected") {
                bigIc = QIcon::fromTheme("network-connect");
            } else if (category == "network.disconnected") {
                bigIc = QIcon::fromTheme("network-disconnect");
            } else if (category == "email.arrived") {
                bigIc = QIcon::fromTheme("mail-receive");
            } else if (category == "battery.charging") {
                bigIc = QIcon::fromTheme("battery-charging-040");
            } else if (category == "battery.charged") {
                bigIc = QIcon::fromTheme("battery-charging-100");
            } else if (category == "battery.discharging") {
                bigIc = QIcon::fromTheme("battery-040");
            } else if (category == "battery.low") {
                bigIc = QIcon::fromTheme("battery-020");
            } else if (category == "battery.critical") {
                bigIc = QIcon::fromTheme("battery-000");
            } else if (category == "device.added") {
                bigIc = QIcon::fromTheme("drive-removable-media");
            } else if (category == "device.removed") {
                bigIc = QIcon::fromTheme("drive-removable-media");
            } else if (category == "call.incoming") {
                bigIc = QIcon::fromTheme("call-start");
            } else if (category == "reminder.activate") {
                bigIc = QIcon::fromTheme("reminder");
            }
        }
    }
}

void NotificationObject::post() {
    dialog->setHints(hints);
    dialog->setApp(appName, appIc);
    dialog->setSummary(summary);
    dialog->setBody(body);
    dialog->setActions(actions);
    dialog->setBigIcon(bigIc);

    if (timeout < 0) {
        timeout = 5000;
    }
    dialog->setTimeout(timeout);

    dialog->show();
}

int NotificationObject::getId() {
    return this->id;
}

void NotificationObject::closeDialog() {
    if (dialog->isVisible()) {
        dialog->close();
    }
}

void NotificationObject::dismiss() {
    closeDialog();

}
