/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#include "notificationdbus.h"

extern QIcon getIconFromTheme(QString name, QColor textColor);
extern TutorialWindow* TutorialWin;
extern AudioManager* AudioMan;

NotificationDBus::NotificationDBus(QObject *parent) : QObject(parent)
{
    new NotificationsAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/freedesktop/Notifications", this);
    dbus.registerService("org.freedesktop.Notifications");
}

QStringList NotificationDBus::GetCapabilities() {
    return QStringList() << "body" << "body-markup" << "icon-static" << "persistence" << "actions" << "sound";
}

uint NotificationDBus::Notify(QString app_name, uint replaces_id,
                              QString app_icon, QString summary,
                              QString body, QStringList actions,
                              QVariantMap hints, int expire_timeout) {\
    NotificationDialog::notificationType type = NotificationDialog::normalType;
    bool showDialog = true;

    if (app_name == "tsbt") { //Do special notifications for ts-bt
        if (hints.value("call-incoming", false).toBool()) {
            summary = "Incoming Call (" + hints.value("device").toString() + ")";
            body = hints.value("caller").toString();
            hints.insert("category", "call.incoming");
            type = NotificationDialog::callType;
            expire_timeout = 0;
        }
    } else if (app_name == "KDE Connect") { //Do special notifications for KDE Connect
        QString kdeEventId = hints.value("x-kde-eventId").toString();
        app_icon = "kdeconnect";
        if (kdeEventId == "pingReceived") {
            body = "Ping received from " + summary;
            summary = "KDE Connect";
        } else if (kdeEventId == "callReceived") {
            QString tempBody, tempSummary;
            tempSummary = "Incoming Call (" + summary + ")";
            tempBody = body.remove("Incoming call from ");
            summary = tempSummary;
            body = tempBody;
            hints.insert("category", "call.incoming");
            type = NotificationDialog::callType;
            expire_timeout = 30000;
        } else if (kdeEventId == "missedCall") {
            showDialog = false;
        } else if (kdeEventId == "smsReceived") {
            summary = "KDE Connect";
        } else if (kdeEventId == "notification") {
            QString appName = body.left(body.indexOf(":"));
            QString appSummary = body.mid(body.indexOf(":") + 2, body.indexOf("‐") - body.indexOf(":") - 3);
            QString appBody = body.right(body.indexOf("‐") + 12);
            summary = appSummary + " (" + appName + " from " + summary + ")";
            body = appBody;
            hints.insert("transient", true);
        } else if (kdeEventId == "batteryLow") {
            summary = summary.remove(": Low Battery");
            body = "The battery is low. Connect the device to power.";
            expire_timeout = 10000;
            hints.insert("category", "battery.low");
        }
    }

    if (replaces_id == 0 || dialogs.count() <= replaces_id) {
        replaces_id = nextId;
        nextId++;

        if (AudioMan != NULL) {
            if (AudioMan->QuietMode() == AudioManager::notifications) {
                QStringList allowedCategories;
                allowedCategories.append("battery.low");
                allowedCategories.append("battery.critical");
                allowedCategories.append("reminder.activate");
                if (!allowedCategories.contains(hints.value("category").toString()) && !hints.value("x-thesuite-timercomplete", false).toBool()) {
                    showDialog = false;
                }
            } else if (AudioMan->QuietMode() == AudioManager::mute) {
                showDialog = false;
            }
        }


        NotificationDialog *d = new NotificationDialog(app_name, app_icon, summary, body, actions, replaces_id, hints, expire_timeout, type);
        d->dbusParent = this;

        connect(d, SIGNAL(closing(int, int)), this, SLOT(sendCloseNotification(int, int)));

        d->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        if (showDialog) {
            if (currentDialog != NULL) {
                currentDialog->close(1);
                QTimer::singleShot(600, [=]() {
                   d->show();
                   currentDialog = d;
                });
            } else {
                d->show();
                currentDialog = d;
            }
            connect(currentDialog, &NotificationDialog::closing, [=]() {
                if (currentDialog == d) {
                    currentDialog = NULL;
                }
            });
        }
        dialogs.append(d);

        //Send the notification to the lock screen if the user desires
        if (settings.value("notifications/lockScreen").toString() != "none") {
            //If the notification is transient, don't send it to the lock screen
            if (!hints.value("transient", false).toBool()) {
                //Create a DBus message relaying the message to the lock screen
                QDBusMessage NotificationEmit = QDBusMessage::createMethodCall("org.thesuite.tsscreenlock", "/org/thesuite/tsscreenlock", "org.thesuite.tsscreenlock.Notifications", "newNotification");
                QVariantList NotificationArgs;

                if (settings.value("notifications/lockScreen", "noContents").toString() == "contents") {
                    NotificationArgs.append(summary);
                    NotificationArgs.append(body);
                    NotificationArgs.append(replaces_id);
                    NotificationArgs.append(actions);
                } else {
                    NotificationArgs.append(app_name);
                    NotificationArgs.append("New Notification");
                    NotificationArgs.append(replaces_id);
                    NotificationArgs.append(QStringList());
                }
                NotificationArgs.append(hints);

                NotificationEmit.setArguments(NotificationArgs);
                QDBusConnection::sessionBus().call(NotificationEmit, QDBus::NoBlock);
            }
        }
    } else {
        dialogs.at(replaces_id - 1)->setParams(app_name, summary, body);
        dialogs.at(replaces_id - 1)->show();
    }

    if (!hints.value("transient", false).toBool()) {
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
            } else if (category == "device.added") {
                icon = getIconFromTheme("connect.svg", color).pixmap(24, 24);
            } else if (category == "device.removed") {
                icon = getIconFromTheme("disconnect.svg", color).pixmap(24, 24);
            } else if (category == "device.removed") {
                icon = getIconFromTheme("disconnect.svg", color).pixmap(24, 24);
            } else if (category == "reminder.activate") {
                icon = getIconFromTheme("reminder", color).pixmap(24, 24);
            }
        } else if (hints.keys().contains("urgency")) {
            QChar urgency = hints.value("urgency").toChar();
            if (urgency.unicode() == 0) {
                icon = QIcon::fromTheme("dialog-information");
            } else if (urgency.unicode() == 2) {
                icon = QIcon::fromTheme("dialog-error");
            }
        }
        emit newNotification(replaces_id, summary, body, icon);
    }

    if (!hints.value("suppress-sound", false).toBool() && !(AudioMan->QuietMode() == AudioManager::notifications || AudioMan->QuietMode() == AudioManager::mute)) {
        if (hints.contains("sound-file")) {
            if (settings.value("notifications/attenuate", true).toBool()) {
                AudioMan->attenuateStreams();
            }

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
                        AudioMan->restoreStreams();
                    }
                }
            });
        } else {
            if (settings.value("notifications/attenuate", true).toBool()) {
                AudioMan->attenuateStreams();
            }

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
                    AudioMan->restoreStreams();
                });
            }
        }
    }
    return replaces_id;
}

QString NotificationDBus::GetServerInformation(QString &vendor, QString &version, QString &spec_version) {

    vendor = "theSuite";
    version = "1.0";
    spec_version = "1.2";
    return "theShell";
}

void NotificationDBus::CloseNotification(uint id) {
    if (id != 0) {
        if (dialogs.count() >= id) {
            NotificationDialog *d = dialogs.at(id - 1);
            d->close(3);
        }
    }
}

void NotificationDBus::CloseNotificationUserInitiated(int id) {
    NotificationDialog *d = dialogs.at(id - 1);
    d->close(2);
}

void NotificationDBus::sendCloseNotification(int id, int reason) {
    emit NotificationClosed((uint) id, (uint) reason);

    if (reason == 2 || reason == 3) {
        emit removeNotification(id);
    }

    if (reason == 1) {
        TutorialWin->showScreen(TutorialWindow::MissedNotification);
    }
}

void NotificationDBus::invokeAction(uint id, QString key) {
    emit ActionInvoked(id, key);
    NotificationDialog *d = dialogs.at(id - 1);
    d->close(2);
}

void NotificationDBus::setDropdownPane(InfoPaneDropdown *pane) {
    this->dropdownPane = pane;
}
