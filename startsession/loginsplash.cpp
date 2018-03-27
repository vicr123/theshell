/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

    ui->label_3->setPixmap(QIcon(":/icons/icon.svg").pixmap(128, 128));
    ui->label_3->setFixedSize(128, 128);
}

LoginSplash::~LoginSplash()
{
    delete ui;
}

void LoginSplash::question(QString title, QString message) {
    ui->questionTitle->setText(title);
    ui->questionText->setText(message);
    ui->stack->setCurrentIndex(1);
}

void LoginSplash::on_yesButton_clicked()
{
    emit response("yes\n");
    ui->stack->setCurrentIndex(0);
}

void LoginSplash::on_noButton_clicked()
{
    emit response("no\n");
    ui->stack->setCurrentIndex(0);
}
