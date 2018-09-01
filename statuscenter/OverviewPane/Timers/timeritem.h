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
        explicit TimerItem(QString timerName, int seconds, QWidget *parent = nullptr);
        ~TimerItem();

    signals:
        void elapsed(QString timerName);

    private:
        Ui::TimerItem *ui;

        tVariantAnimation* progressAnim;

        void paintEvent(QPaintEvent* event);
        void resizeEvent(QResizeEvent* event);
};

#endif // TIMERITEM_H
