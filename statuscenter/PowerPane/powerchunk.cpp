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
#include "powerchunk.h"
#include "ui_powerchunk.h"

#include <QTimer>
#include <QIcon>
#include <the-libs_global.h>
#include <UPower/desktopupower.h>

struct PowerChunkPrivate {
    DesktopUPower* daemon;
    QLabel* snackWidget;

    bool ready = false;
};

PowerChunk::PowerChunk(DesktopUPower*daemon, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PowerChunk)
{
    ui->setupUi(this);

    d = new PowerChunkPrivate();
    d->daemon = daemon;

    d->snackWidget = new QLabel();

    connect(d->daemon, &DesktopUPower::overallStateChanged, this, &PowerChunk::updateChunk);

    QTimer::singleShot(0, this, [=] {
        d->ready = true;
        this->updateChunk();
    });
}

PowerChunk::~PowerChunk()
{
    delete d;
    delete ui;
}

QWidget*PowerChunk::snackWidget()
{
    return d->snackWidget;
}

void PowerChunk::updateChunk()
{
    if (!d->ready) return;

    this->setVisible(d->daemon->shouldShowOverallState());
    QPixmap px = d->daemon->overallStateIcon().pixmap(SC_DPI_T(QSize(16, 16), QSize));

    ui->iconLabel->setPixmap(px);
    d->snackWidget->setPixmap(px);
    ui->descriptionLabel->setText(d->daemon->overallStateDescription());
}

void PowerChunk::mousePressEvent(QMouseEvent*event)
{
    emit activated();
}
