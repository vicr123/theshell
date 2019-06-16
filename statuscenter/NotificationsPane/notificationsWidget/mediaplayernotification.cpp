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

#include <application.h>
#include <QDBusPendingCallWatcher>
#include <the-libs_global.h>

struct MediaPlayerNotificationPrivate {
    QNetworkAccessManager mgr;
    QPalette defaultPal;

    MprisPlayerPtr service;
};

MediaPlayerNotification::MediaPlayerNotification(MprisPlayerPtr service, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MediaPlayerNotification)
{
    ui->setupUi(this);
    d = new MediaPlayerNotificationPrivate();

    d->defaultPal = this->palette();
    d->service = service;

    QIcon appIc = QIcon::fromTheme("generic-app");
    Application app(d->service->desktopEntry());
    if (app.isValid()) {
        appIc = QIcon::fromTheme(app.getProperty("Icon").toString());
    } else if (QIcon::hasThemeIcon(d->service->identity().toLower().replace(" ", "-"))) {
        appIc = QIcon::fromTheme(d->service->identity().toLower().replace(" ", "-"));
    } else if (QIcon::hasThemeIcon(d->service->identity().toLower().replace(" ", ""))) {
        appIc = QIcon::fromTheme(d->service->identity().toLower().replace(" ", ""));
    }
    ui->appIcon->setPixmap(appIc.pixmap(24, 24));

    ui->closeButton->setEnabled(d->service->canQuit());
    ui->appName->setText(d->service->identity());
    setDetails(d->service->metadata().value("xesam:title", d->service->identity()).toString(),
               d->service->metadata().value("xesam:artist").toStringList().join(", "),
               d->service->metadata().value("xesam:album").toString(),
               d->service->metadata().value("xesam:albumArt").toString());
    ui->position->setMaximum(static_cast<int>(d->service->metadata().value("mpris:length").toUInt()));
    ui->position->setEnabled(d->service->canSeek());
    updatePosition(d->service->position());

    if (d->service->playbackStatus() == MprisPlayer::Playing) {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }

    connect(d->service.data(), &MprisPlayer::canQuitChanged, this, [=] {
        ui->closeButton->setEnabled(d->service->canQuit());
    });
    connect(d->service.data(), &MprisPlayer::identityChanged, this, [=] {
        ui->appName->setText(d->service->identity());
    });
    connect(d->service.data(), &MprisPlayer::metadataChanged, this, [=] {
        setDetails(d->service->metadata().value("xesam:title", d->service->identity()).toString(),
                   d->service->metadata().value("xesam:artist").toStringList().join(", "),
                   d->service->metadata().value("xesam:album").toString(),
                   d->service->metadata().value("mpris:artUrl").toString());

        ui->position->setMaximum(static_cast<int>(d->service->metadata().value("mpris:length").toUInt()));
    });
    connect(d->service.data(), &MprisPlayer::playbackStatusChanged, this, [=] {
        if (d->service->playbackStatus() == MprisPlayer::Playing) {
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
        } else {
            ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
        }
    });
    connect(d->service.data(), &MprisPlayer::canSeekChanged, this, [=] {
        ui->position->setEnabled(d->service->canSeek());
    });
    connect(d->service.data(), &MprisPlayer::seeked, this, [=](qint64 position) {
        updatePosition(position);
    });
    connect(d->service.data(), &MprisPlayer::gone, this, &MediaPlayerNotification::deleteLater);

    QTimer* t = new QTimer(this);
    t->setInterval(1000);
    connect(t, &QTimer::timeout, this, QOverload<>::of(&MediaPlayerNotification::updatePosition));
    t->start();
}

MediaPlayerNotification::~MediaPlayerNotification()
{
    delete d;
    delete ui;
}

//void MediaPlayerNotification::updateMpris(QString interfaceName, QMap<QString, QVariant> properties, QStringList changedProperties) {
//    if (interfaceName == "org.mpris.MediaPlayer2.Player") {
//        if (properties.keys().contains("Metadata")) {
//            QVariantMap replyData;
//            properties.value("Metadata").value<QDBusArgument>() >> replyData;

//            QString album = "";
//            QString artist = "";
//            QString title = "";
//            QString albumArt = "";

//            if (replyData.contains("xesam:title")) {
//                title = replyData.value("xesam:title").toString();
//            } else {
//                QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
//                IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

//                QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
//                title = reply.value().variant().toString();
//            }

//            if (replyData.contains("xesam:artist")) {
//                QStringList artists = replyData.value("xesam:artist").toStringList();
//                for (QString art : artists) {
//                    artist.append(art + ", ");
//                }
//                artist.remove(artist.length() - 2, 2);
//            }

//            if (replyData.contains("xesam:album")) {
//                album = replyData.value("xesam:album").toString();
//            }

//            if (replyData.contains("mpris:artUrl")) {
//                albumArt = replyData.value("mpris:artUrl").toString();
//            }

//            if (replyData.contains("mpris:length")) {
//                ui->position->setMaximum(replyData.value("mpris:length").toUInt());
//            }

//            if (replyData.contains("mpris:trackid")) {
//                trackId = replyData.value("mpris:trackid").value<QDBusObjectPath>();
//            }

//            setDetails(title, artist, album, albumArt);
//        }

//        if (properties.keys().contains("PlaybackStatus")) {
//            playbackStatus = properties.value("PlaybackStatus").toString();
//            if (playbackStatus == "Playing") {
//                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
//            } else {
//                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
//            }
//        }

//        if (properties.keys().contains("Rate")) {
//            rate = properties.value("Rate").toDouble();
//        }

//        if (properties.keys().contains("CanSeek")) {
//            ui->position->setEnabled(properties.value("CanSeek").toBool());
//        }
//    }

//}

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
    this->setPalette(d->defaultPal);
    if (albumArt != "") {
        QNetworkRequest req((QUrl(albumArt)));
        QNetworkReply* reply = d->mgr.get(req);
        connect(reply, &QNetworkReply::finished, [=] {
            if (reply->error() == QNetworkReply::NoError) {
                QImage image = QImage::fromData(reply->readAll());
                if (!image.isNull()) {
                    image = image.scaled(48 * theLibsGlobal::getDPIScaling(), 48 * theLibsGlobal::getDPIScaling(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

                    qulonglong red = 0, green = 0, blue = 0;

                    QPalette pal = d->defaultPal;
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
    d->service->previous();
}

void MediaPlayerNotification::on_playPauseButton_clicked()
{
    d->service->playPause();
}

void MediaPlayerNotification::on_nextButton_clicked()
{
    d->service->next();
}

void MediaPlayerNotification::on_closeButton_clicked()
{
    d->service->quit();
}

void MediaPlayerNotification::updatePosition() {
    if (d->service->playbackStatus() == MprisPlayer::Playing) {
        ui->position->blockSignals(true);
        int currentValue = ui->position->value();
        ui->position->setValue(currentValue + qRound((d->service->rate() * 1000000)));
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
    d->service->setPosition(value);
}
