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

#include "loginsplash.h"
#include "ui_loginsplash.h"

LoginSplash::LoginSplash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginSplash)
{
    ui->setupUi(this);

    ui->label_3->setPixmap(QIcon(":/icons/icon.svg").pixmap(128 * theLibsGlobal::instance()->getDPIScaling(), 128 * theLibsGlobal::instance()->getDPIScaling()));
    ui->label_3->setFixedSize(128 * theLibsGlobal::instance()->getDPIScaling(), 128 * theLibsGlobal::instance()->getDPIScaling());

    opacityEffect = new QGraphicsOpacityEffect();
    ui->mainFrame->setGraphicsEffect(opacityEffect);
}

LoginSplash::~LoginSplash()
{
    delete ui;
}

void LoginSplash::question(QString title, QString message) {
    ui->questionTitle->setText(title);
    ui->questionText->setText(message);
    ui->yesButton->setText(tr("Yes"));
    ui->noButton->setText(tr("No"));
    ui->promptLine->setVisible(false);
    ui->stack->setCurrentIndex(1);
}

void LoginSplash::prompt(QString title, QString message) {
    ui->questionTitle->setText(title);
    ui->questionText->setText(message);
    ui->yesButton->setText(tr("OK"));
    ui->noButton->setText(tr("Cancel"));
    ui->promptLine->setVisible(true);
    ui->stack->setCurrentIndex(1);

    ui->promptLine->setFocus();
}

void LoginSplash::on_yesButton_clicked()
{
    if (ui->promptLine->isVisible()) {
        emit response(ui->promptLine->text() + "\n");
    } else {
        emit response("yes\n");
    }
    ui->stack->setCurrentIndex(0);
}

void LoginSplash::on_noButton_clicked()
{
    if (ui->promptLine->isVisible()) {
        emit response("[can]\n");
    } else {
        emit response("no\n");
    }
    ui->stack->setCurrentIndex(0);
}

void LoginSplash::on_promptLine_returnPressed()
{
    ui->yesButton->click();
}

void LoginSplash::show() {
    opacityEffect->setOpacity(1);
    this->setWindowOpacity(1);
    QDialog::show();

    this->setGeometry(QApplication::primaryScreen()->geometry());
    this->showFullScreen();
}

void LoginSplash::hide() {
    if (this->isVisible()) {
        tPropertyAnimation* anim = new tPropertyAnimation(opacityEffect, "opacity");
        anim->setStartValue((float) 1);
        anim->setEndValue((float) 0);
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();
        connect(anim, &tPropertyAnimation::finished, [=] {
            anim->deleteLater();

            tPropertyAnimation* anim = new tPropertyAnimation(this, "windowOpacity");
            anim->setStartValue((float) 1);
            anim->setEndValue((float) 0);
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->start();

            connect(anim, &tPropertyAnimation::finished, [=] {
                anim->deleteLater();

                QDialog::hide();
            });
        });
    }
}
