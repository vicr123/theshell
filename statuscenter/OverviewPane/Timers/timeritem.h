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
