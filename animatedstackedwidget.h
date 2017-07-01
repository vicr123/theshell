#ifndef ANIMATEDSTACKEDWIDGET_H
#define ANIMATEDSTACKEDWIDGET_H

#include <QStackedWidget>
#include <tpropertyanimation.h>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include <QWindow>
#include <QDebug>
#include <QApplication>
#include "upowerdbus.h"

class AnimatedStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AnimatedStackedWidget(QWidget *parent = 0);

signals:
    void switchingFrame(int switchTo);

public slots:
    void setCurrentIndex(int index, bool doAnimation = true);
    void setCurrentWidget(QWidget* w, bool doAnimation = true);

private:
    void doSetCurrentIndex(int index);
    bool doingNewAnimation = false;
    QSettings settings;
};

#endif // ANIMATEDSTACKEDWIDGET_H
