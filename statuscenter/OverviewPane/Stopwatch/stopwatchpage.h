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

#ifndef STOPWATCHPAGE_H
#define STOPWATCHPAGE_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>

namespace Ui {
    class StopwatchPage;
}

class StopwatchPage : public QWidget
{
        Q_OBJECT

    public:
        explicit StopwatchPage(QWidget *parent = nullptr);
        ~StopwatchPage();

    private slots:
        void on_startButton_clicked();

        void on_resetButton_clicked();

    private:
        Ui::StopwatchPage *ui;

        void changeEvent(QEvent* event);

        QDateTime startTime;
        int msecsPadding = 0;
        QTimer* timer;
};

#endif // STOPWATCHPAGE_H
