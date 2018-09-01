#ifndef TIMERPAGE_H
#define TIMERPAGE_H

#include <QStackedWidget>
#include "timeritem.h"

namespace Ui {
    class TimerPage;
}

class TimerPage : public QStackedWidget
{
        Q_OBJECT

    public:
        explicit TimerPage(QWidget *parent = nullptr);
        ~TimerPage();

    private slots:
        void on_backButton_clicked();

        void on_newTimerButton_clicked();

        void on_setTimerButton_clicked();

        void on_newTimerButtonTop_clicked();

        void timerElapsed(QString timerName);

        void notificationClosed(uint id, uint reason);

    private:
        Ui::TimerPage *ui;

        int timersCreated = 0;
        uint currentTimerId = 0;
        QStringList timersElapsed;

        QDBusInterface* notificationInterface;
};

#endif // TIMERPAGE_H
