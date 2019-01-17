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

#ifndef PRINTERMANAGEMENT_H
#define PRINTERMANAGEMENT_H

#include <QWidget>
#include <QListWidgetItem>
#include <cups/cups.h>
#include <statuscenterpaneobject.h>

namespace Ui {
    class PrinterManagement;
}

class PrinterManagement : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit PrinterManagement(QWidget *parent = 0);
        ~PrinterManagement();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_mainMenuButton_clicked();

    private:
        Ui::PrinterManagement *ui;

        int destCount;
        cups_dest_t* dests;
};

#endif // PRINTERMANAGEMENT_H
