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
#ifndef SIMPINPANE_H
#define SIMPINPANE_H

#include <QWidget>
#include <modem.h>

namespace Ui {
    class SimPinPane;
}

struct SimPinPanePrivate;
class SimPinPane : public QWidget
{
        Q_OBJECT

    public:
        explicit SimPinPane(ModemManager::Modem::Ptr modem, QWidget *parent = nullptr);
        ~SimPinPane();

    private slots:
        void on_backButton_clicked();

        void on_enablePinButton_clicked();

        void on_disablePinButton_clicked();

        void on_enablePinButtonGo_clicked();

        void on_stackedWidget_currentChanged(int arg1);

        void on_backButton_2_clicked();

        void on_changePinButtonGo_clicked();

        void on_changePinButton_clicked();

        void on_changePinField_returnPressed();

        void on_enablePinField_returnPressed();

    private:
        Ui::SimPinPane *ui;
        SimPinPanePrivate* d;

        void reloadSimStatus();
};

#endif // SIMPINPANE_H
