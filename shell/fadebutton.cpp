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

#include "fadebutton.h"

extern float getDPIScaling();

FadeButton::FadeButton(QWidget *parent) : QPushButton(parent)
{
    this->setFixedWidth(0);
    this->setMouseTracking(true);
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
    this->setText(fullText);

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(qMin(this->sizeHint().width(), (int) (250 * getDPIScaling())));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(deleteLater()));
    anim->start();
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
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(deleteLater()));
    anim->start();

    if (!animating) {
        //Correct the rectangle's location
        QRect rect = QRect(0, 0, this->width(), this->height());
        textRect.moveLeft(rect.left() + (rect.width() / 2) - (this->fontMetrics().width(currentText) / 2));
        textRect.setWidth(this->fontMetrics().width(currentText));
        textRect.setHeight(this->fontMetrics().height());
        textRect.moveTop(rect.top() + (rect.height() / 2) - (this->fontMetrics().height() / 2));
    }
}

void FadeButton::animateOut() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(deleteLater()));
    anim->start();
}

void FadeButton::paintEvent(QPaintEvent *event) {
    QPalette pal = this->palette();
    QPainter painter(this);
    QBrush brush;
    QPen textPen;

    QRect rect = QRect(0, 0, this->width(), this->height());

    if (this->isEnabled()) {
        brush = QBrush(pal.color(QPalette::Button));
    } else {
        brush = QBrush(pal.color(QPalette::Disabled, QPalette::Button));
    }

    if (this->hasFocus()) {
        brush = QBrush(pal.color(QPalette::Button).lighter(125));
    }

    if (hovering) {
        brush = QBrush(pal.color(QPalette::Button).lighter());
    }

    if (this->isDown() || this->isChecked()) {
        brush = QBrush(pal.color(QPalette::Button).darker(150));
    }
    textPen = pal.color(QPalette::ButtonText);

    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    painter.drawRect(rect);

    QRect iconRect;

    if (!this->icon().isNull()) {
        //int fullWidth = textRect.width() + this->iconSize().width();
        int iconLeft = rect.left() + 8 * getDPIScaling(); //rect.left() + (rect.width() / 2) - (fullWidth / 2);

        iconRect.setLeft(iconLeft);
        iconRect.setTop(rect.top() + (rect.height() / 2) - (this->iconSize().height() / 2));
        iconRect.setSize(this->iconSize());

        //textRect.adjust(button->iconSize.width() / 2, 0, 0, 0);
        textRect.moveLeft(iconRect.right() + 4 * getDPIScaling());
        oldTextRect.moveLeft(iconRect.right() + 4 * getDPIScaling());

        QIcon icon = this->icon();
        QImage image = icon.pixmap(this->iconSize()).toImage();
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

        painter.drawImage(iconRect, image);
    }

    //Draw text
    painter.setPen(textPen);
    painter.setBrush(Qt::transparent);
    painter.drawText(oldTextRect, Qt::AlignLeft, oldText.remove("&"));
    painter.drawText(textRect, Qt::AlignLeft, currentText.remove("&"));

    QLinearGradient endGrad;
    endGrad.setStart(this->width() - 25 * getDPIScaling(), 0);
    endGrad.setFinalStop(this->width(), 0);
    endGrad.setColorAt(0, Qt::transparent);
    endGrad.setColorAt(1, brush.color());
    painter.setBrush(endGrad);
    painter.setPen(Qt::transparent);
    painter.drawRect(rect);
}

void FadeButton::setText(QString text) {
    QRect rect = QRect(0, 0, this->width(), this->height());
    if (currentText == text.remove("&")) {
        if (!animating) {
            //Correct the rectangle's location
            textRect.moveLeft(rect.left() + (rect.width() / 2) - (this->fontMetrics().width(text) / 2));
            textRect.setWidth(this->fontMetrics().width(text));
            textRect.setHeight(this->fontMetrics().height());
            textRect.moveTop(rect.top() + (rect.height() / 2) - (this->fontMetrics().height() / 2));
        }
    } else {
        //Place rectangles in animating position
        oldText = currentText;
        currentText = text;

        oldTextRect.moveLeft(rect.left() + (rect.width() / 2) - (this->fontMetrics().width(oldText) / 2));
        oldTextRect.setWidth(this->fontMetrics().width(oldText));
        oldTextRect.setHeight(this->fontMetrics().height());
        oldTextRect.moveTop(rect.top() + (rect.height() / 2) - (this->fontMetrics().height() / 2));

        int spacing = oldTextRect.top();

        textRect.moveLeft(oldTextRect.left());
        textRect.setWidth(this->fontMetrics().width(text));
        textRect.setHeight(oldTextRect.height());
        textRect.moveTop(oldTextRect.bottom() + spacing);

        //Animate the text
        tVariantAnimation* animation = new tVariantAnimation();
        animation->setStartValue(rect.top() + (rect.height() / 2) - (this->fontMetrics().height() / 2));
        animation->setEndValue(rect.top() + (rect.height() - 3 * this->fontMetrics().height()) / 2 - spacing);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        animation->setDuration(500);
        connect(animation, &tVariantAnimation::valueChanged, [=](QVariant value) {
            oldTextRect.moveTop(value.toInt());
            textRect.moveTop(oldTextRect.bottom() + spacing);
            this->repaint();
        });
        connect(animation, &tVariantAnimation::finished, [=] {
            //Clear animating flag
            animating = false;
        });
        connect(this, SIGNAL(destroyed(QObject*)), animation, SLOT(deleteLater()));
        connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
        animation->start();

        //Set animating flag
        animating = true;
    }
    QPushButton::setText(text);
}

void FadeButton::enterEvent(QEvent* event) {
    hovering = true;
}

void FadeButton::leaveEvent(QEvent *event) {
    hovering = false;
}
