#ifndef REMINDERSPAGE_H
#define REMINDERSPAGE_H

#include <QStackedWidget>
#include <ttoast.h>
#include <tnotification.h>
#include "reminderslistmodel.h"
#include <QDBusInterface>

namespace Ui {
    class RemindersPage;
}

class RemindersPage : public QStackedWidget
{
        Q_OBJECT

    public:
        explicit RemindersPage(QWidget *parent = nullptr);
        ~RemindersPage();

    public slots:
        void updateReminders();

    private slots:
        void on_backButton_clicked();

        void on_addButton_clicked();

        void on_addReminderButton_clicked();

        void on_deleteButton_clicked();

        void checkReminders();

    private:
        Ui::RemindersPage *ui;

        void changeEvent(QEvent* event);

        RemindersListModel* model;
        QDBusInterface* notificationInterface;
};

#endif // REMINDERSPAGE_H
