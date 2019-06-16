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
#include "mprisengine.h"

#include <QTimer>
#include <QDBusConnectionInterface>

struct MprisEnginePrivate {
    MprisEngine* instance = nullptr;
    QMap<QString, MprisPlayerPtr> players;
};

MprisEnginePrivate* MprisEngine::d = new MprisEnginePrivate;

MprisEngine::MprisEngine(QObject *parent) : QObject(parent)
{
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &MprisEngine::serviceOwnerChanged);
    for (QString service : QDBusConnection::sessionBus().interface()->registeredServiceNames().value()) {
        if (service.startsWith("org.mpris.MediaPlayer2.")) registerPlayer(service);
    }
}

MprisEngine* MprisEngine::instance() {
    if (d->instance == nullptr) d->instance = new MprisEngine();
    return d->instance;
}

void MprisEngine::serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner) {
    if (!serviceName.startsWith("org.mpris.MediaPlayer2.")) return; //Not interested in this service
    if (newOwner != "") {
        //Add this player
        QTimer::singleShot(0, [=] {
            registerPlayer(serviceName);
        });
    }
}

MprisPlayerPtr MprisEngine::playerForInterface(QString interface) {
    //Create the instance if it's not been created
    MprisEngine::instance();

    if (d->players.contains(interface)) {
        return d->players.value(interface);
    } else {
        return nullptr;
    }
}

void MprisEngine::registerPlayer(QString service) {
    MprisPlayerPtr player(new MprisPlayer(service));
    d->players.insert(service, player);
    connect(player.get(), &MprisPlayer::gone, this, [=] {
        d->players.remove(service);
        emit playerGone(service);
    });
    emit newPlayer(service, player);
}

QList<MprisPlayerPtr> MprisEngine::players() {
    //Create the instance if it's not been created
    MprisEngine::instance();

    return d->players.values();
}
