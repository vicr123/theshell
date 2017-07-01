#include "animatedstackedwidget.h"

extern UPowerDBus* updbus;

AnimatedStackedWidget::AnimatedStackedWidget(QWidget *parent) : QStackedWidget(parent)
{

}

void AnimatedStackedWidget::setCurrentIndex(int index, bool doAnimation) {
    if (doAnimation) {
        //Do some checks before setting the current index.
        if (currentIndex() != index && !doingNewAnimation) {
            doSetCurrentIndex(index);
        }
    } else {
        QStackedWidget::setCurrentIndex(index);
        doingNewAnimation = false;

        emit switchingFrame(index);
    }
}

void AnimatedStackedWidget::doSetCurrentIndex(int index) {
    //Check if Power Stretch is on
    if (updbus != NULL && updbus->powerStretch()) {
        //Forego animations; power stretch is on
        QStackedWidget::setCurrentIndex(index);
    } else {
        //Forcibly set the current index.
        QWidget* currentWidget = widget(currentIndex());
        QWidget* nextWidget = widget(index);
        if (nextWidget == NULL) {
            QStackedWidget::setCurrentIndex(index);
        } else {
            QRect newGeometry;
            if (currentIndex() < index) {
                nextWidget->setGeometry(this->width() / 2, 0, this->width(), this->height());
                newGeometry.setRect(-this->width() / 8, 0, this->width(), this->height());
            } else {
                nextWidget->setGeometry(-this->width() / 2, 0, this->width(), this->height());
                newGeometry.setRect(this->width() / 8, 0, this->width(), this->height());
            }

            nextWidget->show();
            nextWidget->raise();

            QParallelAnimationGroup* group = new QParallelAnimationGroup;

            QGraphicsOpacityEffect* opacity = new QGraphicsOpacityEffect;
            opacity->setOpacity(0);
            nextWidget->setGraphicsEffect(opacity);

            tPropertyAnimation* opacityAnimation = new tPropertyAnimation(opacity, "opacity");
            opacityAnimation->setStartValue((float) 0);
            opacityAnimation->setEndValue((float) 1);
            opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
            opacityAnimation->setDuration(250);
            group->addAnimation(opacityAnimation);

            tPropertyAnimation* animation = new tPropertyAnimation(nextWidget, "geometry");
            animation->setStartValue(nextWidget->geometry());
            animation->setEndValue(QRect(0, 0, this->width(), this->height()));
            animation->setEasingCurve(QEasingCurve::OutCubic);
            animation->setDuration(250);
            group->addAnimation(animation);

            tPropertyAnimation* animation2 = new tPropertyAnimation(currentWidget, "geometry");
            animation2->setStartValue(currentWidget->geometry());
            animation2->setEndValue(newGeometry);
            animation2->setEasingCurve(QEasingCurve::OutCubic);
            animation2->setDuration(250);
            group->addAnimation(animation2);

            connect(group, &QParallelAnimationGroup::finished, [=]() {
                QStackedWidget::setCurrentIndex(index);
                doingNewAnimation = false;

                opacity->deleteLater();
            });
            group->start();
        }
    }
    emit switchingFrame(index);
}

void AnimatedStackedWidget::setCurrentWidget(QWidget *w, bool doAnimation) {
    this->setCurrentIndex(indexOf(w), doAnimation);
}
