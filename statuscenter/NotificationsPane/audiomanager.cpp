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
#include "audiomanager.h"

#include <QVariant>
#include <QTimer>

AudioManager* AudioManager::i = nullptr;

AudioManager::AudioManager(QObject *parent) : QObject(parent)
{
    quietModeWatcher = new QTimer();
    quietModeWatcher->setInterval(1000);
    connect(quietModeWatcher, &QTimer::timeout, [=] {
        if (QDateTime::currentDateTime() > quietModeOff) {
            setQuietMode(none);
            quietModeWatcher->stop();
        }
    });
}

QWidget* AudioManager::mainWidget() {
    return nullptr;
}


QString AudioManager::name() {
    return "Notifications-AudioManagerHelper";
}

StatusCenterPaneObject::StatusPaneTypes AudioManager::type() {
    return None;
}

int AudioManager::position() {
    return 0;
}

void AudioManager::message(QString name, QVariantList args) {
    if (name == "quiet-mode-changed") {
        currentMode = (quietMode) args.first().toInt();
        emit QuietModeChanged(currentMode);
    }
}

AudioManager::quietMode AudioManager::QuietMode() {
    return currentMode;
}

AudioManager* AudioManager::instance() {
    if (AudioManager::i == nullptr) {
        AudioManager::i = new AudioManager();
    }
    return AudioManager::i;
}

void AudioManager::attenuateStreams() {

}

void AudioManager::restoreStreams() {

}

void AudioManager::setQuietMode(quietMode mode) {
    sendMessage("set-quiet-mode", {mode});
}

void AudioManager::setQuietModeResetTime(QDateTime time) {
    if (time.isValid()) {
        quietModeOff = time;
        quietModeWatcher->start();
    } else {
        quietModeWatcher->stop();
    }
}
