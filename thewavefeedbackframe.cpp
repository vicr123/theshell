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
        painter.drawLine(i, this->height(), i, this->height() - (level * this->height()));
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
