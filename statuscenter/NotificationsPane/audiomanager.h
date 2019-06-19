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
#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <statuscenterpaneobject.h>
#include <QDateTime>
#include <debuginformationcollector.h>

class AudioManager : public QObject, public StatusCenterPaneObject
{
        Q_OBJECT
    public:
        explicit AudioManager(QObject *parent = T_QOBJECT_ROOT);
        static AudioManager* instance();

        enum quietMode {
            none,
            critical,
            notifications,
            mute
        };

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args = QVariantList());

        quietMode QuietMode();
    signals:
        void QuietModeChanged(quietMode mode);

    public slots:
        void attenuateStreams();
        void restoreStreams();
        void setQuietMode(quietMode mode);
        void setQuietModeResetTime(QDateTime time);

    private:
        quietMode currentMode = none;

        QDateTime quietModeOff;
        QTimer* quietModeWatcher;

        static AudioManager* i;
};

#endif // AUDIOMANAGER_H
