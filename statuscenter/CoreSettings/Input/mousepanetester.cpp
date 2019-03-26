/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "mousepanetester.h"

#include <QSvgRenderer>
#include <QPainter>
#include <the-libs_global.h>

struct MousePaneTesterPrivate {
    QSvgRenderer* renderer;
};

MousePaneTester::MousePaneTester(QWidget *parent) : QWidget(parent)
{
    d = new MousePaneTesterPrivate();
    d->renderer = new QSvgRenderer(QLatin1Literal(":/images/mouseclicktest.svg"));
}

MousePaneTester::~MousePaneTester() {
    d->renderer->deleteLater();
    delete d;
}

void MousePaneTester::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    d->renderer->render(&painter, QRect(0, 0, this->width(), this->height()));
}

QSize MousePaneTester::sizeHint() const {
    QSize sz = d->renderer->viewBox().size();
    sz.scale(450 * theLibsGlobal::getDPIScaling(), 0, Qt::KeepAspectRatioByExpanding);
    return sz;
}
