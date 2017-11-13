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

#include "loginsplash.h"
#include "ui_loginsplash.h"

LoginSplash::LoginSplash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginSplash)
{
    ui->setupUi(this);

    ui->label_3->setPixmap(QIcon(":/icons/icon.svg").pixmap(512, 512));

    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(5000);
    connect(timer, SIGNAL(timeout()), this, SLOT(close()));
    timer->start();
}

LoginSplash::~LoginSplash()
{
    delete ui;
}
