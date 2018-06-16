#include "mediaplayernotification.h"
#include "ui_mediaplayernotification.h"

MediaPlayerNotification::MediaPlayerNotification(QString service, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MediaPlayerNotification)
{
    ui->setupUi(this);

    QDBusConnection::sessionBus().connect(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateMpris(QString,QMap<QString, QVariant>,QStringList)));
    this->service = service;

    //Get Quit Capability
    QDBusMessage QuitRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    QuitRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "CanQuit");

    QDBusReply<QVariant> quit(QDBusConnection::sessionBus().call(QuitRequest));
    ui->closeButton->setEnabled(quit.value().toBool());

    //Get app name
    QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

    QDBusReply<QDBusVariant> nameReply(QDBusConnection::sessionBus().call(IdentityRequest));
    QString appName = nameReply.value().variant().toString();
    ui->appName->setText(appName);

    //Get app icon
    QDBusMessage DesktopRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    DesktopRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "DesktopEntry");

    QDBusReply<QDBusVariant> DesktopReply(QDBusConnection::sessionBus().call(DesktopRequest));
    QString icon = DesktopReply.value().variant().toString();

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

    //Get Current Song Metadata
    QDBusMessage MetadataRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    MetadataRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Metadata");

    QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(MetadataRequest));
    QVariantMap replyData;
    QDBusArgument arg(reply.value().variant().value<QDBusArgument>());

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

    setDetails(title, artist, album, albumArt);


    //Get Playback Status
    QDBusMessage PlayStatRequest = QDBusMessage::createMethodCall(service, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    PlayStatRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "PlaybackStatus");

    QDBusReply<QVariant> PlayStat(QDBusConnection::sessionBus().call(PlayStatRequest));
    if (PlayStat.value().toString() == "Playing") {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    }
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

            setDetails(title, artist, album, albumArt);
        }

        if (properties.keys().contains("PlaybackStatus")) {
            if (properties.value("PlaybackStatus").toString() == "Playing") {
                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-pause"));
            } else {
                ui->playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
            }
        }
    }

}

void MediaPlayerNotification::setDetails(QString title, QString artist, QString album, QString albumArt) {
    ui->detailsLabel->setText(title);

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
    if (albumArt != "") {
        QNetworkRequest req((QUrl(albumArt)));
        QNetworkReply* reply = mgr.get(req);
        connect(reply, &QNetworkReply::finished, [=] {
            if (reply->error() == QNetworkReply::NoError) {
                QImage image = QImage::fromData(reply->readAll());
                if (!image.isNull()) {
                    ui->albumArt->setPixmap(QPixmap::fromImage(image).scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
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
