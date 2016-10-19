#include "animatedstackedwidget.h"

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index) {
    //Do some checks before setting the current index.
    if (currentIndex() != index && !doingNewAnimation) {
        doSetCurrentIndex(index);
    }
}

void AnimatedStackedWidget::doSetCurrentIndex(int index) {
    //Forcibly set the current index.
    QWidget* currentWidget = widget(currentIndex());
    QWidget* nextWidget = widget(index);
    if (nextWidget == NULL) {
        QStackedWidget::setCurrentIndex(index);
    } else {
        if (currentIndex() < index) {
            nextWidget->setGeometry(this->width(), 0, this->width(), this->height());
        } else {
            nextWidget->setGeometry(-this->width(), 0, this->width(), this->height());
        }

        nextWidget->show();
        nextWidget->raise();

        QSequentialAnimationGroup* group = new QSequentialAnimationGroup;

        QPropertyAnimation* animation = new QPropertyAnimation(nextWidget, "geometry");
        animation->setStartValue(nextWidget->geometry());
        animation->setEndValue(QRect(0, 0, this->width(), this->height()));
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->setDuration(250);
        group->addAnimation(animation);

        connect(group, &QSequentialAnimationGroup::finished, [=]() {
            QStackedWidget::setCurrentIndex(index);
            doingNewAnimation = false;
        });
        group->start();
    }
    emit switchingFrame(index);
}

void AnimatedStackedWidget::setCurrentWidget(QWidget *w) {
    this->setCurrentIndex(indexOf(w));
}
