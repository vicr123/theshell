/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef USERSPANE_H
#define USERSPANE_H

#include <QWidget>
#include <tpromise.h>
#include <statuscenterpaneobject.h>

namespace Ui {
    class UsersPane;
}

struct UsersPanePrivate;
class UsersPane : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit UsersPane(QWidget *parent = nullptr);
        ~UsersPane();

        QWidget*mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_mainMenuButton_clicked();

        void on_addButton_clicked();

        void on_deleteUserButton_clicked();

    private:
        Ui::UsersPane *ui;
        UsersPanePrivate* d;

        tPromise<void>* checkPolkit(bool isOwnUser);
        void currentUserChanged();
};

#endif // USERSPANE_H
