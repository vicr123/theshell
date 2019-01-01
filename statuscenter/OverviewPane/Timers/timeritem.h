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

#ifndef TIMERITEM_H
#define TIMERITEM_H

#include <QWidget>
#include <QPainter>
#include <tvariantanimation.h>

namespace Ui {
    class TimerItem;
}

class TimerItem : public QWidget
{
        Q_OBJECT

    public:
        explicit TimerItem(QString timerName, int seconds, QString ringtone, QWidget *parent = nullptr);
        ~TimerItem();

    signals:
        void elapsed(QString timerName, QString ringtone);

    private slots:
        void on_pauseButton_clicked();

    private:
        Ui::TimerItem *ui;

        tVariantAnimation* progressAnim;

        void paintEvent(QPaintEvent* event);
        void resizeEvent(QResizeEvent* event);
};

#endif // TIMERITEM_H
