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

#include "mousescrollwidget.h"

MouseScrollWidget::MouseScrollWidget(QWidget *parent) : QScrollArea(parent)
{
    this->setMouseTracking(true);
    shiftLeft.setInterval(10);
    connect(&shiftLeft, &QTimer::timeout, [=]() {
        if (QApplication::isRightToLeft()) {
            this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + scrollSpeed);
        } else {
            this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - scrollSpeed);
        }
    });

    shiftRight.setInterval(10);
    connect(&shiftRight, &QTimer::timeout, [=]() {
        if (QApplication::isRightToLeft()) {
            this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - scrollSpeed);
        } else {
            this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + scrollSpeed);
        }
    });

    connect(this->horizontalScrollBar(), &QScrollBar::valueChanged, [=]() {
        vp->repaint();
    });
    //vp->installEventFilter(this);

    QTimer* eventFilterWait = new QTimer();
    eventFilterWait->setInterval(1000);
    connect(eventFilterWait, &QTimer::timeout, [=]() {
        setEventFilter(this->widget());
        this->setFixedHeight(this->sizeHint().height());
    });
    eventFilterWait->start();
}

void MouseScrollWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->x() < 100) {
        shiftLeft.start();
        shiftRight.stop();
    } else if (event->x() > this->width() - 100) {
        shiftLeft.stop();
        shiftRight.start();
    } else {
        shiftLeft.stop();
        shiftRight.stop();
    }
}

void MouseScrollWidget::wheelEvent(QWheelEvent *event) {
    //this->scroll(event->delta(), 0);
    if (event->orientation() == Qt::Vertical) {
        //It's more natural to scroll downwards to go to the right, so subtract delta rather than add.
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - event->delta());
    }
}

void MouseScrollWidget::setWidget(QWidget *widget) {
    QScrollArea::setWidget(widget);
    vp = widget;
    widget->setAttribute(Qt::WA_PaintUnclipped);
    this->installEventFilter(widget);
}

void MouseScrollWidget::setEventFilter(QWidget *widget) {
    if (widget != NULL) {
        widget->setMouseTracking(true);
        widget->installEventFilter(this);

        /*for (QObject* child : widget->children()) {
            ((QWidget*) child)->installEventFilter(this);
            for (QObject* child1 : child->children()) {
                ((QWidget*) child1)->installEventFilter(this);
                for (QObject* child2 : child->children()) {
                    ((QWidget*) child2)->installEventFilter(this);
                    for (QObject* child3 : child->children()) {
                        ((QWidget*) child3)->installEventFilter(this);
                    }
                }
            }
        }*/
        /*for (QObject* child : widget->children()) {
            setEventFilter(static_cast<QWidget*>(child));
        }*/
        setEventFilter(widget->layout());
    }
}

void MouseScrollWidget::setEventFilter(QLayout *layout) {
    if (layout != NULL) {
        for (int i = 0; i < layout->count(); i++) {
            if (layout->itemAt(i)->widget() == NULL) {
                setEventFilter(layout->itemAt(i)->layout());
            } else {
                setEventFilter(layout->itemAt(i)->widget());
            }
        }
    }
}

bool MouseScrollWidget::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::Enter) {
        QMouseEvent* mouseEvent = (QMouseEvent*) event;
        QPoint point = ((QWidget*) object)->mapTo(this, mouseEvent->pos());

        if (point.x() < 10) {
            shiftLeft.start();
            shiftRight.stop();
            scrollSpeed = 5;
        } else if (point.x() < 50) {
            shiftLeft.start();
            shiftRight.stop();
            scrollSpeed = 2;
        } else if (point.x() < 100) {
            shiftLeft.start();
            shiftRight.stop();
            scrollSpeed = 1;
        } else if (point.x() > this->width() - 10) {
            shiftLeft.stop();
            shiftRight.start();
            scrollSpeed = 5;
        } else if (point.x() > this->width() - 50) {
            shiftLeft.stop();
            shiftRight.start();
            scrollSpeed = 2;
        } else if (point.x() > this->width() - 100) {
            shiftLeft.stop();
            shiftRight.start();
            scrollSpeed = 1;
        } else { //Stop scrolling
            shiftLeft.stop();
            shiftRight.stop();
        }
    } else if (event->type() == QEvent::Leave) {
        shiftLeft.stop();
        shiftRight.stop();
    /*} else if (event->type() == QEvent::Scroll) {
        QScrollEvent* scrollEvent = (QScrollEvent*) event;

        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + scrollEvent->contentPos().y());*/
    /*} else if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = (QWheelEvent*) event;

        //this->scroll(wheelEvent->delta(), 0);
        if (wheelEvent->orientation() == Qt::Vertical) {
            this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + wheelEvent->delta() / 8 / 15);
        }*/
    } else if ((event->type() == QEvent::Paint) && (object == vp)) {
        QPainter painter(vp);

        painter.setPen(QColor(0, 0, 0, 0));
        int leftStart = this->horizontalScrollBar()->value() * 2;

        if (this->horizontalScrollBar()->maximum() != 0) {
            painter.setBrush(this->palette().brush(QPalette::Highlight));
            int thisWidth = this->width() - 18;
            int left, width;
            qreal scaleFactor = (qreal) thisWidth / (qreal) (thisWidth + this->horizontalScrollBar()->maximum());
            width = (qreal) thisWidth * scaleFactor;

            qreal ratio = (qreal) this->horizontalScrollBar()->value() / (qreal) this->horizontalScrollBar()->maximum();
            left = this->horizontalScrollBar()->value() * ratio;

            if (settings.value("bar/onTop").toBool()) {
                painter.drawRect(leftStart + left, 0, width, 4);
            } else {
                painter.drawRect(leftStart + left, this->height() - 4, width, 4);
            }
        }
        return true;
    }
    return false;
}

QSize MouseScrollWidget::sizeHint() const {
    QSize hint = QScrollArea::sizeHint();
    hint.setHeight(widget()->sizeHint().height());
    return hint;
}

void MouseScrollWidget::paintEvent(QPaintEvent *event) {
    //QPainter painter(this);
    //painter.setBrush(this->palette().brush(QPalette::WindowText));
    //Check if we need to draw the left scroll indicator
    /*if (this->horizontalScrollBar()->value() != 0) {
        QGradient grad;
        grad.setColorAt(0, this->palette().color(QPalette::WindowText));
        QColor end = this->palette().color(QPalette::WindowText);
        end.setAlpha(0);
        grad.setColorAt(20, QColor(end));
        painter.setBrush(QBrush(grad));
        painter.drawRect(event->rect().left(), event->rect().top(), 20, this->height());
    }

    //Check if we need to draw the right scroll indicator
    if (this->horizontalScrollBar()->value() != this->horizontalScrollBar()->maximum()) {
        painter.drawRect(event->rect().left() + this->width() - 20, event->rect().top(), 20, this->height());
    }*/
}
