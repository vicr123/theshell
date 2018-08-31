#ifndef TIMERPAGE_H
#define TIMERPAGE_H

#include <QStackedWidget>

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

    private:
        Ui::TimerPage *ui;
};

#endif // TIMERPAGE_H
