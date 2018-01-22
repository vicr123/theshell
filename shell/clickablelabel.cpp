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

#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent) :
    QLabel(parent)
{

}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    isClicked = true;
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event) {
    if (didDrag) {
        emit mouseReleased();
        didDrag = false;
    }
    if (event->x() < this->width() && event->y() < this->height()) {
        emit clicked();
    }
    isClicked = false;
    event->accept();
}

bool ClickableLabel::showDisabled() {
    return isShowDisabled;
}

void ClickableLabel::setShowDisabled(bool showDisabled) {
    isShowDisabled = showDisabled;

    QPalette applicationPal;
    if (this->parentWidget() == nullptr) {
        QLabel* tempLabel = new QLabel;
        applicationPal = QApplication::palette(tempLabel);
        tempLabel->deleteLater();
    } else {
        applicationPal = this->parentWidget()->palette();
    }
    QPalette thisPal = this->palette();
    if (isShowDisabled) {
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Disabled, QPalette::WindowText));
    } else {
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Normal, QPalette::WindowText));
    }
    this->setPalette(thisPal);
}

void ClickableLabel::mouseMoveEvent(QMouseEvent *event) {
    if (isClicked) {
        emit dragging(event->x(), event->y());
        didDrag = true;
    }
}
