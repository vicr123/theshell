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

#ifndef TIMERPAGE_H
#define TIMERPAGE_H

#include <QStackedWidget>
#include <QMediaPlayer>
#include "timeritem.h"

namespace Ui {
    class TimerPage;
}

class TimerPage : public QStackedWidget
{
        Q_OBJECT

    public:
        explicit TimerPage(QWidget *parent = nullptr);
        ~TimerPage();

    private slots:
        void on_backButton_clicked();

        void on_newTimerButton_clicked();

        void on_setTimerButton_clicked();

        void on_newTimerButtonTop_clicked();

        void timerElapsed(QString timerName, QString ringtone);

        void notificationClosed(uint id, uint reason);

    signals:
        void attenuate();
        void deattenuate();

    private:
        Ui::TimerPage *ui;

        int timersCreated = 0;
        uint currentTimerId = 0;
        QStringList timersElapsed;

        QMediaPlayer* ringtone;

        void changeEvent(QEvent* event);

        QDBusInterface* notificationInterface;
};

#endif // TIMERPAGE_H
