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

#include "mediaplayernotification.h"
#include "ui_mediaplayernotification.h"

#include <QDBusPendingCallWatcher>
#include <the-libs_global.h>

MediaPlayerNotification::MediaPlayerNotification(QString service, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MediaPlayerNotification)
{
    ui->setupUi(this);

    this->defaultPal = this->palette();

    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateMpris(QString,QMap<QString, QVariant>,QStringList)));
    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Seeked", this, SLOT(updatePosition(qint64)));
    this->service = service;

    //Get Quit Capability
    QDBusMessage QuitRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    QuitRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "CanQuit");

    QDBusReply<QVariant> quit(QDBusConnection::sessionBus().call(QuitRequest, QDBus::Block, 1000));
    ui->closeButton->setEnabled(quit.value().toBool());

    //Get app name
    QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(IdentityRequest));
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        QString appName = watcher->reply().arguments().first().value<QDBusVariant>().variant().toString();
        ui->appName->setText(appName);

        //Get app icon
        QDBusMessage DesktopRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        DesktopRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "DesktopEntry");

        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(DesktopRequest));
        connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
            QString icon = watcher->reply().arguments().first().value<QDBusVariant>().variant().toString();

            QIcon appIc = QIcon::fromTheme("generic-app");
            if (QIcon::hasThemeIcon(appName.toLower().replace(" ", "-"))) {
                appIc = QIcon::fromTheme(appName.toLower().replace(" ", "-"));
            } else if (QIcon::hasThemeIcon(appName.toLower().replace(" ", ""))) {
                appIc = QIcon::fromTheme(appName.toLower().replace(" ", ""));
            } else {
                QDir appFolder("/usr/share/applications/");
                QDirIterator iterator(appFolder, QDirIterator::Subdirectories);

                while (iterator.hasNext()) {
                    iterator.next();
                    QFileInfo info = iterator.fileInfo();
                    if (info.fileName() == icon || info.baseName().toLower() == appName.toLower()) {
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
            ui->appIcon->setPixmap(appIc.pixmap(24, 24));

            watcher->deleteLater();
        });

        //Get Current Song Metadata
        QDBusMessage MetadataRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        MetadataRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Metadata");

        QDBusPendingCallWatcher* metadata = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(MetadataRequest));
        connect(metadata, &QDBusPendingCallWatcher::finished, [=] {
            QVariantMap replyData;
            QDBusArgument arg(metadata->reply().arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>());

            arg >> replyData;

            QString album = "";
            QString artist = "";
            QString title = "";
            QString albumArt;

            if (replyData.contains("xesam:title")) {
                title = replyData.value("xesam:title").toString();
            } else {
                title = appName;
            }

            if (replyData.contains("xesam:artist")) {
                QStringList artists = replyData.value("xesam:artist").toStringList();
                for (QString art : artists) {
                    artist.append(art + ", ");
                }
                artist.remove(artist.length() - 2, 2);
            }

            if (replyData.contains("xesam:album")) {
                album = replyData.value("xesam:album").toString();
            }

            if (replyData.contains("mpris:artUrl")) {
                albumArt = replyData.value("mpris:artUrl").toString();
            }

            if (replyData.contains("mpris:length")) {
                ui->position->setMaximum(replyData.value("mpris:length").toUInt());
            }

            if (replyData.contains("mpris:trackid")) {
                trackId = replyData.value("mpris:trackid").value<QDBusObjectPath>();
            }

            setDetails(title, artist, album, albumArt);
        });
    });
    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, &QDBusPendingCallWatcher::deleteLater);


    //Get Playback Status
    QDBusMessage PlayStatRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    PlayStatRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "PlaybackStatus");

    QDBusReply<QVariant> PlayStat(QDBusConnection::sessionBus().call(PlayStatRequest, QDBus::Block, 1000));
    playbackStatus = PlayStat.value().toString();
    if (playbackStatus == "Playing") {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }

    //Get Rate
    QDBusMessage RateRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    RateRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Rate");

    QDBusReply<QVariant> RateReply(QDBusConnection::sessionBus().call(RateRequest, QDBus::Block, 1000));
    rate = RateReply.value().toDouble();

    //Get Position
    QDBusMessage PosRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    PosRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Position");

    QDBusReply<QVariant> PosReply(QDBusConnection::sessionBus().call(PosRequest, QDBus::Block, 1000));
    ui->position->setValue(PosReply.value().toInt());

    //Get Seekable
    QDBusMessage SeekableRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    SeekableRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "CanSeek");

    QDBusReply<QVariant> SeekableReply(QDBusConnection::sessionBus().call(SeekableRequest, QDBus::Block, 1000));
    ui->position->setEnabled(SeekableReply.value().toBool());

    QTimer* t = new QTimer();
    t->setInterval(1000);
    connect(t, SIGNAL(timeout()), this, SLOT(updatePosition()));
    connect(this, SIGNAL(destroyed(QObject*)), t, SLOT(stop()));
    connect(this, SIGNAL(destroyed(QObject*)), t, SLOT(deleteLater()));
    t->start();
}

MediaPlayerNotification::~MediaPlayerNotification()
{
    delete ui;
}

void MediaPlayerNotification::updateMpris(QString interfaceName, QMap<QString, QVariant> properties, QStringList changedProperties) {
    if (interfaceName == "org.mpris.MediaPlayer2.Player") {
        if (properties.keys().contains("Metadata")) {
            QVariantMap replyData;
            properties.value("Metadata").value<QDBusArgument>() >> replyData;

            QString album = "";
            QString artist = "";
            QString title = "";
            QString albumArt = "";

            if (replyData.contains("xesam:title")) {
                title = replyData.value("xesam:title").toString();
            } else {
                QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
                IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

                QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
                title = reply.value().variant().toString();
            }

            if (replyData.contains("xesam:artist")) {
                QStringList artists = replyData.value("xesam:artist").toStringList();
                for (QString art : artists) {
                    artist.append(art + ", ");
                }
                artist.remove(artist.length() - 2, 2);
            }

            if (replyData.contains("xesam:album")) {
                album = replyData.value("xesam:album").toString();
            }

            if (replyData.contains("mpris:artUrl")) {
                albumArt = replyData.value("mpris:artUrl").toString();
            }

            if (replyData.contains("mpris:length")) {
                ui->position->setMaximum(replyData.value("mpris:length").toUInt());
            }

            if (replyData.contains("mpris:trackid")) {
                trackId = replyData.value("mpris:trackid").value<QDBusObjectPath>();
            }

            setDetails(title, artist, album, albumArt);
        }

        if (properties.keys().contains("PlaybackStatus")) {
            playbackStatus = properties.value("PlaybackStatus").toString();
            if (playbackStatus == "Playing") {
                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            } else {
                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            }
        }

        if (properties.keys().contains("Rate")) {
            rate = properties.value("Rate").toDouble();
        }

        if (properties.keys().contains("CanSeek")) {
            ui->position->setEnabled(properties.value("CanSeek").toBool());
        }
    }

}

void MediaPlayerNotification::setDetails(QString title, QString artist, QString album, QString albumArt) {
    if (ui->detailsLabel->text() != title) {
        updatePosition(0);
        ui->detailsLabel->setText(title);
    }

    if (artist == "" && album == "") {
        ui->supplementaryLabel->setVisible(false);
    } else if (artist != "" && album == ""){
        ui->supplementaryLabel->setVisible(true);
        ui->supplementaryLabel->setText(artist);
    } else if (artist == "" && album != ""){
        ui->supplementaryLabel->setVisible(true);
        ui->supplementaryLabel->setText(album);
    } else if (artist != "" && album != ""){
        ui->supplementaryLabel->setVisible(true);
        ui->supplementaryLabel->setText(artist + " · " + album);
    }

    ui->albumArt->setPixmap(QIcon::fromTheme("audio").pixmap(48, 48));
    this->setPalette(defaultPal);
    if (albumArt != "") {
        QNetworkRequest req((QUrl(albumArt)));
        QNetworkReply* reply = mgr.get(req);
        connect(reply, &QNetworkReply::finished, [=] {
            if (reply->error() == QNetworkReply::NoError) {
                QImage image = QImage::fromData(reply->readAll());
                if (!image.isNull()) {
                    image = image.scaled(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                    qulonglong red = 0, green = 0, blue = 0;

                    QPalette pal = this->defaultPal;
                    int totalPixels = 0;
                    for (int i = 0; i < image.width(); i++) {
                        for (int j = 0; j < image.height(); j++) {
                            QColor c = image.pixelColor(i, j);
                            if (c.alpha() != 0) {
                                red += c.red();
                                green += c.green();
                                blue += c.blue();
                                totalPixels++;
                            }
                        }
                    }

                    QColor c;
                    int averageCol = (pal.color(QPalette::Window).red() + pal.color(QPalette::Window).green() + pal.color(QPalette::Window).blue()) / 3;

                    if (totalPixels == 0) {
                        if (averageCol < 127) {
                            c = pal.color(QPalette::Window).darker(200);
                        } else {
                            c = pal.color(QPalette::Window).lighter(200);
                        }
                    } else {
                        c = QColor(red / totalPixels, green / totalPixels, blue / totalPixels);

                        if (averageCol < 127) {
                            c = c.darker(200);
                        } else {
                            c = c.lighter(200);
                        }
                    }

                    pal.setColor(QPalette::Window, c);
                    this->setPalette(pal);

                    QImage rounded(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), QImage::Format_ARGB32);
                    rounded.fill(Qt::transparent);
                    QPainter p(&rounded);
                    p.setRenderHint(QPainter::Antialiasing);
                    p.setBrush(QBrush(image));
                    p.setPen(Qt::transparent);
                    p.drawRoundedRect(0, 0, 48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), 40, 40, Qt::RelativeSize);

                    ui->albumArt->setPixmap(QPixmap::fromImage(rounded));
                }
            }
            reply->deleteLater();
        });
    }
}

void MediaPlayerNotification::on_backButton_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Previous"), QDBus::NoBlock);
}

void MediaPlayerNotification::on_playPauseButton_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "PlayPause"), QDBus::NoBlock);
}

void MediaPlayerNotification::on_nextButton_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Next"), QDBus::NoBlock);
}

void MediaPlayerNotification::on_closeButton_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Quit"), QDBus::NoBlock);
}

void MediaPlayerNotification::updatePosition() {
    if (playbackStatus == "Playing") {
        ui->position->blockSignals(true);
        int currentValue = ui->position->value();
        ui->position->setValue(currentValue + (rate * 1000000));
        ui->position->blockSignals(false);
    }
}

void MediaPlayerNotification::updatePosition(qint64 position) {
    ui->position->blockSignals(true);
    ui->position->setValue(position);
    ui->position->blockSignals(false);
}

void MediaPlayerNotification::on_position_valueChanged(int value)
{
    QDBusMessage changeMessage = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "SetPosition");
    changeMessage.setArguments(QList<QVariant>() << QVariant::fromValue(trackId) << (qint64) value);
    QDBusConnection::sessionBus().call(changeMessage, QDBus::NoBlock);
}
