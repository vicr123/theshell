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

#include "thewavefeedbackframe.h"

theWaveFeedbackFrame::theWaveFeedbackFrame(QWidget *parent) : QFrame(parent)
{
    connect(&indeterminateTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    indeterminateTimer.setInterval(10);
    for (int i = 0; i < this->width(); i = i + 2) {
        levels.append((qreal) 1 / (qreal) this->height());
    }
}

void theWaveFeedbackFrame::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int i = this->width();
    if (indeterminate) {
        //qreal random = (qreal) qrand() / (RAND_MAX);
        levels.insert(0, (qSin(qDegreesToRadians((qreal) indeterminateStage * 3 - 90))) / 2 + 0.5);
        indeterminateStage++;
    }
    for (qreal level : levels) {
        //painter.drawLine(i, this->height(), i, this->height() - (level * this->height()));
        int distanceFromMiddle = (level * this->height()) / 2;
        painter.drawLine(i, this->height() / 2 + distanceFromMiddle, i, this->height() / 2 - distanceFromMiddle);
        i = i - 2;
    }

    event->accept();
}

void theWaveFeedbackFrame::addLevel(qreal level) {
    if (level == -1) { //Processing
        indeterminate = true;
        indeterminateTimer.start();
    } else if (level == -2) { //Reset
        levels.clear();
        for (int i = 0; i < this->width(); i = i + 2) {
            levels.append((qreal) 1 / (qreal) this->height());
        }
        indeterminateTimer.stop();

    } else {
        levels.insert(0, level);
        this->repaint();
        indeterminate = false;
        indeterminateStage = 0;
        indeterminateTimer.stop();
    }
}
