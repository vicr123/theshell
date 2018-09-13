#ifndef STOPWATCHPAGE_H
#define STOPWATCHPAGE_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>

namespace Ui {
    class StopwatchPage;
}

class StopwatchPage : public QWidget
{
        Q_OBJECT

    public:
        explicit StopwatchPage(QWidget *parent = nullptr);
        ~StopwatchPage();

    private slots:
        void on_startButton_clicked();

        void on_resetButton_clicked();

    private:
        Ui::StopwatchPage *ui;

        void changeEvent(QEvent* event);

        QDateTime startTime;
        int msecsPadding = 0;
        QTimer* timer;
};

#endif // STOPWATCHPAGE_H
