/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef THEWAVEFEEDBACKFRAME_H
#define THEWAVEFEEDBACKFRAME_H

#include <QFrame>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QtMath>

class theWaveFeedbackFrame : public QFrame
{
    Q_OBJECT
public:
    explicit theWaveFeedbackFrame(QWidget *parent = 0);

signals:

public slots:
    void addLevel(qreal level);

private:
    void paintEvent(QPaintEvent *event);

    QList<qreal> levels;
    bool indeterminate = false;
    int indeterminateStage = 0;
    QTimer indeterminateTimer;
};

#endif // THEWAVEFEEDBACKFRAME_H
