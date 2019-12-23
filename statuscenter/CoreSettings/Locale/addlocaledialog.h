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
#ifndef ADDLOCALEDIALOG_H
#define ADDLOCALEDIALOG_H

#include <QWidget>

namespace Ui {
    class AddLocaleDialog;
}

struct AddLocaleDialogPrivate;
class AddLocaleDialog : public QWidget
{
        Q_OBJECT

    public:
        explicit AddLocaleDialog(QWidget *parent = nullptr);
        ~AddLocaleDialog();

    private slots:
        void on_languageList_activated(const QModelIndex &index);

        void on_backButton_clicked();

        void on_backButton_2_clicked();

        void on_regionList_activated(const QModelIndex &index);

    signals:
        void done();

    private:
        Ui::AddLocaleDialog *ui;

        AddLocaleDialogPrivate* d;
};

#endif // ADDLOCALEDIALOG_H
