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

#include "notificationobject.h"

int NotificationObject::currentId = 0;

const QDBusArgument &operator<<(QDBusArgument &argument, const ImageData &d) {
    argument.beginStructure();
    argument << d.width << d.height << d.rowstride << d.alpha << d.bitsPerSample << d.channels << d.data;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ImageData &d) {
    argument.beginStructure();
    argument >> d.width >> d.height >> d.rowstride >> d.alpha >> d.bitsPerSample >> d.channels >> d.data;
    argument.endStructure();
    return argument;
}

NotificationObject::NotificationObject(QString app_name, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout, QObject *parent) : QObject(parent) {
    currentId++;
    this->id = currentId;

    dialog = new NotificationPopup(currentId);
    connect(dialog, SIGNAL(actionClicked(QString)), this, SIGNAL(actionClicked(QString)));
    connect(dialog, &NotificationPopup::notificationClosed, [=](uint reason) {
        emit closed((NotificationCloseReason) reason);
    });

    setParameters(app_name, app_icon, summary, body, actions, hints, expire_timeout);
    this->date = QDateTime::currentDateTimeUtc();
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

    actionNamesAreIcons = hints.value("action-icons", false).toBool();

    /*if (hints.contains("image-data")) {
        QDBusArgument imageDataArg = hints.value("image-data").value<QDBusArgument>();
        ImageData i;
        imageDataArg >> i;
        char* data = i.data.data();

        /*QImage image(i.width, i.height, QImage::Format_ARGB32);
        int lineWidth = i.bitsPerSample * i.channels * i.width;
        for (int x = 0; x < i.height; x++) {
            memcpy(image.scanLine(x), i.data.data() + (lineWidth * x), lineWidth);
        }

        bigIc = QIcon(QPixmap::fromImage(image));

        QImage image(i.width, i.height, QImage::Format_ARGB32);

        for (int y = 0; y < i.height; y++) {
            for (int x = 0; x < i.width; x++) {
                unsigned long a, r, g, b;

                a = (data[(int) (y * i.width + x + 0)]);
                r = (data[(int) (y * i.width + x + 1)]);
                g = (data[(int) (y * i.width + x + 2)]);
                b = (data[(int) (y * i.width + x + 3)]);

                QColor col = QColor(r, g, b, a);

                image.setPixelColor(x, y, col);
            }
        }

        QPixmap iconPixmap(QPixmap::fromImage(image));//.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        bigIc = QIcon(iconPixmap);
    }*/

    emit parametersUpdated();
}

void NotificationObject::post() {
    dialog->setHints(hints);
    dialog->setApp(appName, appIc);
    dialog->setSummary(summary);
    dialog->setBody(body);
    dialog->setActions(actions, actionNamesAreIcons);
    dialog->setBigIcon(bigIc);

    if (timeout < 0) {
        timeout = 5000;
    }
    dialog->setTimeout(timeout);

    if (notificationAppSettings->value(appName + "/popup", true).toBool() && !dialog->isVisible()) {
        dialog->show();
    }

    //Play sounds if requested
    if (!hints.value("suppress-sound", false).toBool() && !(AudioManager::instance()->QuietMode() == AudioManager::notifications || AudioManager::instance()->QuietMode() == AudioManager::mute) && notificationAppSettings->value(appName + "/sounds", true).toBool()) {
        if (settings.value("notifications/attenuate", true).toBool()) {
            AudioManager::instance()->attenuateStreams();
        }

        if (hints.contains("sound-file")) {
            QMediaPlayer* player = new QMediaPlayer();
            if (hints.value("sound-file").toString().startsWith("qrc:")) {
                player->setMedia(QMediaContent(QUrl(hints.value("sound-file").toString())));
            } else {
                player->setMedia(QMediaContent(QUrl::fromLocalFile(hints.value("sound-file").toString())));
            }
            player->play();
            connect(player, &QMediaPlayer::stateChanged, [=](QMediaPlayer::State state) {
                if (state == QMediaPlayer::StoppedState) {
                    player->deleteLater();
                    if (settings.value("notifications/attenuate", true).toBool()) {
                        AudioManager::instance()->restoreStreams();
                    }
                }
            });
        } else {
            QSoundEffect* sound = new QSoundEffect();

            QString notificationSound = settings.value("notifications/sound", "tripleping").toString();
            if (notificationSound == "tripleping") {
                sound->setSource(QUrl("qrc:/sounds/notifications/tripleping.wav"));
            } else if (notificationSound == "upsidedown") {
                sound->setSource(QUrl("qrc:/sounds/notifications/upsidedown.wav"));
            } else if (notificationSound == "echo") {
                sound->setSource(QUrl("qrc:/sounds/notifications/echo.wav"));
            }
            sound->play();
            connect(sound, SIGNAL(playingChanged()), sound, SLOT(deleteLater()));

            if (settings.value("notifications/attenuate", true).toBool()) {
                connect(sound, &QSoundEffect::playingChanged, [=]() {
                    AudioManager::instance()->restoreStreams();
                });
            }
        }
    }
}

uint NotificationObject::getId() {
    return this->id;
}

void NotificationObject::closeDialog() {
    if (dialog->isVisible()) {
        dialog->close();
    }
}

void NotificationObject::dismiss() {
    closeDialog();
    emit closed(Dismissed);
}

QIcon NotificationObject::getAppIcon() {
    return appIc;
}

QString NotificationObject::getAppIdentifier() {
    return this->appName;
}

QString NotificationObject::getAppName() {
    return this->appName;
}

QString NotificationObject::getSummary() {
    return this->summary;
}

QString NotificationObject::getBody() {
    return this->body;
}

QDateTime NotificationObject::getDate() {
    return this->date;
}
