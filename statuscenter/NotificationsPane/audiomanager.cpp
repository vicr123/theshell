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
#include <soundengine.h>

AudioManager* AudioManager::i = nullptr;

AudioManager::AudioManager(QObject *parent) : QObject(parent)
{
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

}

AudioManager* AudioManager::instance() {
    if (AudioManager::i == nullptr) {
        AudioManager::i = new AudioManager();
    }
    return AudioManager::i;
}

void AudioManager::attenuateStreams() {
    sendMessage("attenuate", {true});
}

void AudioManager::restoreStreams() {
    sendMessage("attenuate", {false});
}
