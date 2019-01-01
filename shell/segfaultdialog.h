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

#ifndef SEGFAULTDIALOG_H
#define SEGFAULTDIALOG_H

#include <QDialog>
#include <execinfo.h>
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>

namespace Ui {
class SegfaultDialog;
}

class SegfaultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SegfaultDialog(QString signal, QWidget *parent = 0);
    ~SegfaultDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::SegfaultDialog *ui;
};

#endif // SEGFAULTDIALOG_H
