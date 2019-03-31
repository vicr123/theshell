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
#include "gesturepane.h"
#include "ui_gesturepane.h"

#include <QSettings>
#include <QSvgRenderer>
#include <QPainter>
#include <QDebug>
#include <QTimer>

struct GesturePanePrivate {
    QSettings settings;

    QSvgRenderer *touchModeRenderer, *swipeOpenRenderer;
};

GesturePane::GesturePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GesturePane)
{
    ui->setupUi(this);
    d = new GesturePanePrivate();

    ui->touchModeSwitch->setChecked(d->settings.value("input/touch", false).toBool());
    ui->swipeGatewaySwitch->setChecked(d->settings.value("gestures/swipeGateway", true).toBool());

    d->swipeOpenRenderer = new QSvgRenderer(QLatin1Literal(":/images/gatewayopen-static.svg"));
    d->touchModeRenderer = new QSvgRenderer(QLatin1Literal(":/images/touchmode-static.svg"));

    ui->swipeGatewayAnimation->installEventFilter(this);
    ui->swipeGatewayAnimation->setFixedSize(d->swipeOpenRenderer->viewBox().size() * 2 * theLibsGlobal::getDPIScaling());
    ui->touchModeAnimation->installEventFilter(this);
    ui->touchModeAnimation->setFixedSize(d->touchModeRenderer->viewBox().size() * 2 * theLibsGlobal::getDPIScaling());
}

GesturePane::~GesturePane()
{
    delete ui;
    delete d;
}

void GesturePane::on_touchModeSwitch_toggled(bool checked)
{
    d->settings.setValue("input/touch", checked);
}

bool GesturePane::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Paint) {
        QPainter painter;
        painter.begin(static_cast<QWidget*>(watched));
        if (watched == ui->swipeGatewayAnimation) {
            d->swipeOpenRenderer->render(&painter, QRectF(0, 0, ui->swipeGatewayAnimation->width(), ui->swipeGatewayAnimation->height()));
        } else if (watched == ui->touchModeAnimation) {
            d->touchModeRenderer->render(&painter, QRectF(0, 0, ui->touchModeAnimation->width(), ui->touchModeAnimation->height()));
        }
        painter.end();
        return true;
    }
    return false;
}

void GesturePane::on_swipeGatewaySwitch_toggled(bool checked)
{
    d->settings.setValue("gestures/swipeGateway", checked);
}

void GesturePane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
