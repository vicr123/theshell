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
#ifndef REDSHIFTENGINE_H
#define REDSHIFTENGINE_H

#include <QObject>
#include <debuginformationcollector.h>

struct RedshiftEnginePrivate;
class RedshiftEngine : public QObject
{
        Q_OBJECT
    public:
        explicit RedshiftEngine(QObject *parent = T_QOBJECT_ROOT);
        ~RedshiftEngine();

        bool isEnabled();

    signals:
        void redshiftEnabledChanged(bool enabled);

    public slots:
        void processTimer();
        void preview(int temp);
        void endPreview();
        void overrideRedshift(bool enabled);

    private slots:
        void adjust(int temp);

    private:
        RedshiftEnginePrivate* d;
};

#endif // REDSHIFTENGINE_H
