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

#ifndef STARTMONITOR_H
#define STARTMONITOR_H

#include <QObject>
#include <loginsplash.h>
#include <QApplication>

class StartMonitor : public QObject
{
        Q_OBJECT
    public:
        explicit StartMonitor(QObject *parent = nullptr);

        bool started();
    signals:
        void questionResponse(QString response);

    public slots:
        void MarkStarted();
        void MarkNotStarted();
        void ShowSplash();
        void HideSplash();

        void SplashQuestion(QString title, QString msg);
        void SplashPrompt(QString title, QString msg);

    private:
        bool s = false;
        LoginSplash* splash;
};

#endif // STARTMONITOR_H
