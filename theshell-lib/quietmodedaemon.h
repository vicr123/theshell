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
#ifndef QUIETMODEDAEMON_H
#define QUIETMODEDAEMON_H

#include <QObject>

struct QuietModeDaemonPrivate;
class QuietModeDaemon : public QObject
{
        Q_OBJECT
    public:
        enum QuietMode {
            None = 0,
            Critical = 1,
            Notifications = 2,
            Mute = 3
        };

        static QuietModeDaemon* instance();

        static QuietMode getQuietMode();
        static void setQuietMode(QuietMode quietMode);
        static void setQuietModeResetTime(QDateTime time);
        static QString getCurrentQuietModeDescription();

    signals:
        void QuietModeChanged(QuietMode newMode, QuietMode oldMode);

    private:
        explicit QuietModeDaemon(QObject *parent = nullptr);
        static QuietModeDaemonPrivate* d;
};

#endif // QUIETMODEDAEMON_H
