/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef MPRISPLAYER_H
#define MPRISPLAYER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QSharedPointer>
#include "debuginformationcollector.h"

typedef QMap<QString, QVariant> MetadataMap;

class QDBusInterface;
struct MprisPlayerPrivate;
class MprisPlayer : public QObject
{
        Q_OBJECT
    public:
        explicit MprisPlayer(QString service, QObject *parent = T_QOBJECT_ROOT);
        ~MprisPlayer();

        enum PlayingStatus {
            Playing,
            Paused,
            Stopped
        };

        enum RepeatStatus {
            NoRepeat,
            RepeatOne,
            RepeatAll
        };

        Q_PROPERTY(QString identity READ identity NOTIFY identityChanged)
        Q_PROPERTY(bool canQuit READ canQuit NOTIFY canQuitChanged)
        Q_PROPERTY(bool canFullscreen READ canFullscreen NOTIFY canFullscreenChanged)
        Q_PROPERTY(bool canRaise READ canRaise NOTIFY canRaiseChanged)
        Q_PROPERTY(bool hasTrackList READ hasTrackList NOTIFY hasTrackListChanged)
        Q_PROPERTY(bool isFullscreen READ isFullscreen WRITE setIsFullscreen NOTIFY isFullscreenChanged)
        Q_PROPERTY(QString desktopEntry READ desktopEntry NOTIFY desktopEntryChanged)

        Q_PROPERTY(PlayingStatus playbackStatus READ playbackStatus NOTIFY playbackStatusChanged)
        Q_PROPERTY(RepeatStatus repeating READ repeating WRITE setRepeating NOTIFY repeatingChanged)
        Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
        Q_PROPERTY(MetadataMap metadata READ metadata NOTIFY metadataChanged)
        Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
        Q_PROPERTY(double minRate READ minRate NOTIFY minRateChanged)
        Q_PROPERTY(double maxRate READ maxRate NOTIFY maxRateChanged)
        Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY canGoNextChanged)
        Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY canGoPreviousChanged)
        Q_PROPERTY(bool canPlay READ canPlay NOTIFY canPlayChanged)
        Q_PROPERTY(bool canPause READ canPause NOTIFY canPauseChanged)
        Q_PROPERTY(bool canSeek READ canSeek NOTIFY canSeekChanged)

        QString service();

        QString identity();
        bool canQuit();
        bool canFullscreen();
        bool canRaise();
        bool hasTrackList();
        bool isFullscreen();
        QString desktopEntry();

        void setIsFullscreen(bool fullscreen);

        PlayingStatus playbackStatus();
        RepeatStatus repeating();
        double rate();
        bool shuffle();
        MetadataMap metadata();
        double volume();
        qint64 position();
        double minRate();
        double maxRate();
        bool canGoNext();
        bool canGoPrevious();
        bool canPlay();
        bool canPause();
        bool canSeek();
        bool canControl();

        void setRepeating(RepeatStatus repeating);
        void setRate(double rate);
        void setShuffle(bool shuffle);
        void setVolume(double volume);

    signals:
        void seeked(qint64 position);
        void identityChanged();
        void canQuitChanged();
        void canFullscreenChanged();
        void canRaiseChanged();
        void hasTrackListChanged();
        void isFullscreenChanged();
        void desktopEntryChanged();

        void playbackStatusChanged();
        void repeatingChanged();
        void shuffleChanged();
        void metadataChanged();
        void volumeChanged();
        void minRateChanged();
        void maxRateChanged();
        void canGoNextChanged();
        void canGoPreviousChanged();
        void canPlayChanged();
        void canPauseChanged();
        void canSeekChanged();
        void canControlChanged();

        void gone();

    public slots:
        void raise();
        void quit();

        void next();
        void previous();
        void pause();
        void playPause();
        void stop();
        void play();
        void seek(qint64 offset);
        void setPosition(qint64 position);
        void openUri(QString uri);

    private slots:
        void dbusPropertyChanged(QString interfaceName, QMap<QString, QVariant> changedProperties, QStringList invalidatedProperties);
        void serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner);

    private:
        MprisPlayerPrivate* d;

        void registerDbusProperty(QDBusInterface* interface, QString localProperty, QString remoteProperty);
        QVariant privateProperty(QString name);
};
typedef QSharedPointer<MprisPlayer> MprisPlayerPtr;

#endif // MPRISPLAYER_H
