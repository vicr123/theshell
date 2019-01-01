/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
