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

#include "taskbarbutton.h"
#include <Wm/desktopwm.h>

extern float getDPIScaling();

struct TaskbarButtonPrivate {
    DesktopWmWindowPtr window;
    bool hovering = false;
    bool pendingDelete = false;

    QString txt;

    QRect oldTextRect, textRect;
    QString oldText = "", currentText = "";

    bool fadeOff = false;
    bool isActive = false;
    bool isWindowMinimized = false;
    bool shouldBeVisible = true;
};

TaskbarButton::TaskbarButton(DesktopWmWindowPtr window) : QPushButton(nullptr)
{
    d = new TaskbarButtonPrivate();
    d->window = window;

    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setFixedWidth(0);
    this->setMouseTracking(true);

    connect(DesktopWm::instance(), &DesktopWm::activeWindowChanged, this, [=] {
        d->isActive = DesktopWm::activeWindow() == d->window;
        this->update();
    });
    connect(window, &DesktopWmWindow::titleChanged, this, [=] {
        this->setText(window->title());
    });
    connect(window, &DesktopWmWindow::iconChanged, this, [=] {
        this->setIcon(window->icon());
    });
    connect(window, &DesktopWmWindow::windowStateChanged, this, [=] {
        d->isWindowMinimized = d->window->isMinimized();
        d->shouldBeVisible = d->window->shouldShowInTaskbar();
        this->update();
        this->adjustSize();
    });
    connect(window, &DesktopWmWindow::destroyed, this, [=] {
        d->pendingDelete = true;
        this->animateOut();
    });

    d->isActive = DesktopWm::activeWindow() == d->window;
    d->isWindowMinimized = d->window->isMinimized();
    d->shouldBeVisible = d->window->shouldShowInTaskbar();
    this->setText(window->title());
    this->setIcon(window->icon());

    connect(this, &TaskbarButton::clicked, this, [=] {
        if (!d->isActive) {
            d->window->activate();
        }
    });
}

void TaskbarButton::animateOut() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    connect(anim, &tVariantAnimation::finished, this, [=] {
        if (d->pendingDelete) this->deleteLater();
    });
    connect(this, &TaskbarButton::destroyed, anim, &tVariantAnimation::deleteLater);
    anim->start();
}

QSize TaskbarButton::sizeHint() const
{
    int width = d->textRect.width() + SC_DPI(18);
    if (!this->icon().isNull()) width += this->iconSize().width() + SC_DPI(6);

    return QSize(width, this->fontMetrics().height() + SC_DPI(18));
}

void TaskbarButton::paintEvent(QPaintEvent *event) {
    QPalette pal = this->palette();
    QPainter painter(this);
    QBrush brush = QBrush(pal.color(QPalette::Button));
    QPen textPen;

    QRect rect = QRect(0, 0, this->width(), this->height());

    if (!this->isEnabled()) brush = QBrush(pal.color(QPalette::Disabled, QPalette::Button));
    if (this->hasFocus()) brush = QBrush(pal.color(QPalette::Button).lighter(125));
    if (d->hovering) brush = QBrush(pal.color(QPalette::Button).lighter());
    if (this->isDown() || d->isActive) brush = QBrush(pal.color(QPalette::Button).darker(150));

    textPen = pal.color(QPalette::ButtonText);

    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    painter.drawRect(rect);

    QRect textRect = d->textRect;
    QRect oldTextRect = d->oldTextRect;

    if (!this->icon().isNull()) {
        QRect iconRect;
        iconRect.setSize(this->iconSize());
        iconRect.moveTop(this->height() / 2 - iconRect.height() / 2);
        iconRect.moveLeft(SC_DPI(9));

        oldTextRect.moveLeft(iconRect.right() + SC_DPI(6));
        textRect.moveLeft(iconRect.right() + SC_DPI(6));

        QIcon icon = this->icon();
        QPixmap px = icon.pixmap(this->height() * 2);

        painter.drawPixmap(iconRect, px);
    }

    //Draw text
    painter.setPen(textPen);
    painter.setBrush(Qt::transparent);
    painter.save();
    if (d->isWindowMinimized) painter.setOpacity(0.5);
    painter.drawText(oldTextRect, Qt::AlignLeft, d->oldText);
    painter.drawText(textRect, Qt::AlignLeft, d->currentText);
    painter.restore();

    if (d->fadeOff) {
        QLinearGradient endGrad;
        endGrad.setStart(this->width() - SC_DPI(25), 0);
        endGrad.setFinalStop(this->width(), 0);
        endGrad.setColorAt(0, Qt::transparent);
        endGrad.setColorAt(1, brush.color());
        painter.setBrush(endGrad);
        painter.setPen(Qt::transparent);
        painter.drawRect(rect);
    }
}

void TaskbarButton::setText(QString text) {
    QRect textRect;
    textRect.setHeight(fontMetrics().height());
    textRect.setWidth(fontMetrics().horizontalAdvance(text));
    textRect.moveTop(this->sizeHint().height() / 2 - textRect.height() / 2);
    textRect.moveLeft(textRect.top());

    d->oldText = this->text();
    d->oldTextRect = d->textRect;
    d->textRect = textRect;

    d->oldText = d->currentText;
    d->currentText = text;

    tVariantAnimation* oldAnim = new tVariantAnimation();
    oldAnim->setStartValue(d->oldTextRect);
    oldAnim->setEndValue(d->oldTextRect.translated(0, -d->oldTextRect.bottom()));
    oldAnim->setDuration(500);
    oldAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(oldAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->oldTextRect = value.toRect();
        this->update();
    });
    connect(oldAnim, &tVariantAnimation::finished, oldAnim, &tVariantAnimation::deleteLater);
    oldAnim->start();

    tVariantAnimation* newAnim = new tVariantAnimation();
    newAnim->setStartValue(textRect.translated(0, this->sizeHint().height() - textRect.top()));
    newAnim->setEndValue(textRect);
    newAnim->setDuration(500);
    newAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(newAnim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        d->textRect = value.toRect();
        this->update();
    });
    connect(newAnim, &tVariantAnimation::finished, newAnim, &tVariantAnimation::deleteLater);
    newAnim->start();

    this->adjustSize();
}

void TaskbarButton::setIcon(QIcon icon)
{
    QPushButton::setIcon(icon);
    this->adjustSize();
}

void TaskbarButton::adjustSize()
{
    int width;
    if (d->shouldBeVisible) {
        width = this->sizeHint().width();
        if (width > SC_DPI(250)) {
            width = SC_DPI(250);
            d->fadeOff = true;
        } else {
            d->fadeOff = false;
        }
    } else {
        width = 0;
    }

    if (this->width() != width) {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(this->width());
        anim->setEndValue(qMin(this->sizeHint().width(), SC_DPI(250)));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            this->setFixedWidth(value.toInt());
        });
        connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
        connect(this, &TaskbarButton::destroyed, anim, &tVariantAnimation::deleteLater);
        anim->start();
    }
}

void TaskbarButton::enterEvent(QEvent* event) {
    d->hovering = true;
}

void TaskbarButton::leaveEvent(QEvent *event) {
    d->hovering = false;
}
