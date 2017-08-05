/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef LOGINSPLASH_H
#define LOGINSPLASH_H

#include <QDialog>
#include <QTimer>
#include <QLabel>
#include <QIcon>

namespace Ui {
class LoginSplash;
}

class LoginSplash : public QDialog
{
    Q_OBJECT

public:
    explicit LoginSplash(QWidget *parent = 0);
    ~LoginSplash();

private:
    Ui::LoginSplash *ui;
};

#endif // LOGINSPLASH_H
