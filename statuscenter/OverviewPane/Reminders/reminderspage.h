#ifndef REMINDERSPAGE_H
#define REMINDERSPAGE_H

#include <QStackedWidget>

namespace Ui {
    class RemindersPage;
}

class RemindersPage : public QStackedWidget
{
        Q_OBJECT

    public:
        explicit RemindersPage(QWidget *parent = nullptr);
        ~RemindersPage();

    private:
        Ui::RemindersPage *ui;
};

#endif // REMINDERSPAGE_H
