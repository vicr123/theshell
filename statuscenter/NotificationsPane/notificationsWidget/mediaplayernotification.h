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

#ifndef MEDIAPLAYERNOTIFICATION_H
#define MEDIAPLAYERNOTIFICATION_H

#include <QFrame>
#include <QDBusConnection>
#include <QDBusReply>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QSlider>
#include <QPainter>

namespace Ui {
    class MediaPlayerNotification;
}

class MediaPlayerNotification : public QFrame
{
        Q_OBJECT

    public:
        explicit MediaPlayerNotification(QString service, QWidget *parent = 0);
        ~MediaPlayerNotification();

    public slots:
        void updateMpris(QString interfaceName, QMap<QString, QVariant> properties, QStringList changedProperties);
        void setDetails(QString title, QString artist, QString album, QString albumArt);

    private slots:
        void on_backButton_clicked();

        void on_playPauseButton_clicked();

        void on_nextButton_clicked();

        void on_closeButton_clicked();

        void updatePosition();

        void updatePosition(qint64 position);

        void on_position_valueChanged(int value);

    private:
        Ui::MediaPlayerNotification *ui;
        QString service;
        QNetworkAccessManager mgr;
        double rate;
        QString playbackStatus;
        QDBusObjectPath trackId;

        QPalette defaultPal;
};

#endif // MEDIAPLAYERNOTIFICATION_H
