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
#ifndef GESTUREPANE_H
#define GESTUREPANE_H

#include <QWidget>

namespace Ui {
    class GesturePane;
}

struct GesturePanePrivate;
class GesturePane : public QWidget
{
        Q_OBJECT

    public:
        explicit GesturePane(QWidget *parent = nullptr);
        ~GesturePane();

    private slots:
        void on_touchModeSwitch_toggled(bool checked);

    private:
        Ui::GesturePane *ui;

        GesturePanePrivate* d;
};

#endif // GESTUREPANE_H
