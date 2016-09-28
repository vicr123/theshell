#include "mousescrollwidget.h"

MouseScrollWidget::MouseScrollWidget(QWidget *parent) : QScrollArea(parent)
{
    this->setMouseTracking(true);
    shiftLeft.setInterval(10);
    connect(&shiftLeft, &QTimer::timeout, [=]() {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - scrollSpeed);
    });

    shiftRight.setInterval(10);
    connect(&shiftRight, &QTimer::timeout, [=]() {
       this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + scrollSpeed);
    });

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

void MouseScrollWidget::setWidget(QWidget *widget) {
    QScrollArea::setWidget(widget);

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
        qDebug() << mouseEvent->x() << mouseEvent->y();

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
    } else if (event->type() == QEvent::Scroll) {
        QScrollEvent* scrollEvent = (QScrollEvent*) event;

        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + scrollEvent->contentPos().y());
    }
    return false;
}

QSize MouseScrollWidget::sizeHint() const {
    QSize hint = QScrollArea::sizeHint();
    //hint.setHeight(hint.height() - QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    return hint;
}
