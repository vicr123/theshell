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

#include "fadebutton.h"

FadeButton::FadeButton(QWidget *parent) : QPushButton(parent)
{
    this->setFixedWidth(0);
}

bool FadeButton::fade() {
    return f;
}

void FadeButton::setFade(bool fade) {
    f = fade;
    QPalette pal = this->palette();
    QColor col = pal.color(QPalette::ButtonText);
    if (fade) {
        col.setAlpha(127);
    } else {
        col.setAlpha(255);
    }
    pal.setBrush(QPalette::ButtonText, QBrush(col));
    this->setPalette(pal);
    this->repaint();
}

void FadeButton::setFullText(QString fullText) {
    this->txt = fullText;
    QFontMetrics metrics(this->font());
    this->setText(metrics.elidedText(fullText, Qt::ElideRight, 200));
}

void FadeButton::animateIn() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(this->sizeHint().width());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void FadeButton::animateOut() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}
