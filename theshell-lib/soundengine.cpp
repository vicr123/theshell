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
#include "soundengine.h"

#include <QSettings>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <tsystemsound.h>

struct SoundEnginePrivate {
    QMediaPlayer* player;
};

SoundEngine::SoundEngine(QObject *parent) : QObject(parent)
{
    d = new SoundEnginePrivate();
}

SoundEngine* SoundEngine::play(QUrl path, qreal volume) {
    SoundEngine* engine = new SoundEngine();

    QMediaPlayer* player = new QMediaPlayer(engine, QMediaPlayer::LowLatency);
    player->setAudioRole(QAudio::NotificationRole);
    player->setMedia(path);
    player->setVolume(qRound(QAudio::convertVolume(volume, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale) * 100));
    connect(player, &QMediaPlayer::stateChanged, engine, [=](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            //We're done here
            emit engine->done();

            player->deleteLater();
            engine->deleteLater();
        }
    });
    player->play();

    return engine;
}

SoundEngine* SoundEngine::play(QString soundName, qreal volume) {
    //Play a sound from the audio theme
    tSystemSound* sound = tSystemSound::play(soundName, volume);
    if (sound == nullptr) {
        return nullptr;
    } else {
        SoundEngine* engine = new SoundEngine();
        connect(sound, &tSystemSound::done, engine, [=] {
            //We're done here
            emit engine->done();
            engine->deleteLater();
        });
        return engine;
    }
}

SoundEngine* SoundEngine::play(KnownSound sound, qreal volume) {
    QSettings settings;
    switch (sound) {
        case Notification: {
            QString notificationSound = settings.value("notifications/sound", "tripleping").toString();
            if (notificationSound == "tripleping") {
                return play(QUrl("qrc:/sounds/notifications/tripleping.wav"), volume);
            } else if (notificationSound == "upsidedown") {
                return play(QUrl("qrc:/sounds/notifications/upsidedown.wav"), volume);
            } else if (notificationSound == "echo") {
                return play(QUrl("qrc:/sounds/notifications/echo.wav"), volume);
            } else {
                return nullptr;
            }
        }
        case Volume:
            return playKnownSound("audio-volume-change", "volfeedback", volume);
        case Login:
            return playKnownSound("desktop-login", "login", volume);
        case Logout:
            return playKnownSound("desktop-logout", "logout", volume);
        case Screenshot:
            return playKnownSound("screen-capture", "screenshot", volume);
    }

    return nullptr;
}

SoundEngine* SoundEngine::playKnownSound(QString soundName, QString soundSetting, qreal volume) {
    if (tSystemSound::isSoundEnabled(soundSetting)) {
        return play(soundName, volume);
    } else {
        return nullptr;
    }
}
