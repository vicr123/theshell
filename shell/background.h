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

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QDialog>
#include <QSettings>
#include <QMenu>
#include <QDesktopWidget>
#include <QSvgRenderer>
#include "mainwindow.h"
#include "choosebackground.h"

#include <X11/Xlib.h>

class ChooseBackground;

namespace Ui {
class Background;
}

class Background : public QDialog
{
    Q_OBJECT

public:
    explicit Background(MainWindow* mainwindow, QRect screenGeometry, QWidget *parent = 0);
    ~Background();

    void show();

private slots:
    void on_graphicsView_customContextMenuRequested(const QPoint &pos);

    void on_actionOpen_Status_Center_triggered();

    void on_actionOpen_theShell_Settings_triggered();

    void on_actionOpen_System_Settings_triggered();

    void on_actionChange_Background_triggered();

private:
    Ui::Background *ui;

    void reject();

    MainWindow* mainwindow;
};

#endif // BACKGROUND_H
