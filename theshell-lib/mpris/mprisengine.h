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
#ifndef MPRISENGINE_H
#define MPRISENGINE_H

#include <QObject>
#include "mprisplayer.h"

struct MprisEnginePrivate;
class MprisEngine : public QObject
{
        Q_OBJECT
    public:
        static MprisEngine* instance();

        static MprisPlayerPtr playerForInterface(QString interface);
        static QList<MprisPlayerPtr> players();

    signals:
        void newPlayer(QString service, MprisPlayerPtr player);
        void playerGone(QString service);

    private slots:
        void serviceOwnerChanged(QString serviceName, QString oldOwner, QString newOwner);

    private:
        explicit MprisEngine(QObject *parent = nullptr);
        static MprisEnginePrivate* d;

        void registerPlayer(QString service);
};

#endif // MPRISENGINE_H
