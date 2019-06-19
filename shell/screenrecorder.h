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

#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QObject>
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QX11Info>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <debuginformationcollector.h>

class ScreenRecorder : public QObject
{
        Q_OBJECT
    public:
        explicit ScreenRecorder(QObject *parent = T_QOBJECT_ROOT);

        enum State {
            Idle,
            Recording,
            Processing
        };

    signals:
        void stateChanged(State state);


    public slots:
        void start();
        void stop();
        bool recording();

    private slots:
        void recorderFinished(int returnCode);

    private:
        QProcess* recorderProcess;
        State s = Idle;
};

#endif // SCREENRECORDER_H
