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

#ifndef LOCATIONREQUESTDIALOG_H
#define LOCATIONREQUESTDIALOG_H

#include <QDialog>
#include <QIcon>
#include <QLabel>

namespace Ui {
    class LocationRequestDialog;
}

class LocationRequestDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit LocationRequestDialog(QWidget *parent = 0);
        ~LocationRequestDialog();

    public slots:
        void setAppName(QString appName);
        void setIcon(QIcon icon);
        void setReason(QString reason);

    private slots:
        void on_denyButton_clicked();

        void on_allowButton_clicked();

    private:
        Ui::LocationRequestDialog *ui;
        QString appName;
};

#endif // LOCATIONREQUESTDIALOG_H
