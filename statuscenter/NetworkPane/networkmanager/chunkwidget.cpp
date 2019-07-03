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
#include "chunkwidget.h"
#include "ui_chunkwidget.h"

#include "devicepanel.h"
#include <QPointer>
#include <QIcon>
#include <the-libs_global.h>

struct ChunkWidgetPrivate {
    QPointer<DevicePanel> watchingPanel;
    QMetaObject::Connection watchConnection;
    QLabel* snackWidget;

    QString supplementaryText;

    bool isFlightMode = false;
};

ChunkWidget::ChunkWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChunkWidget)
{
    ui->setupUi(this);
    d = new ChunkWidgetPrivate();
    d->snackWidget = new QLabel();
}

ChunkWidget::~ChunkWidget()
{
    d->snackWidget->deleteLater();
    delete ui;
    delete d;
}

void ChunkWidget::setIcon(QIcon icon, bool isFlightMode) {
    ui->iconLabel->setPixmap(icon.pixmap(SC_DPI_T(QSize(16, 16), QSize)));
    d->snackWidget->setPixmap(icon.pixmap(SC_DPI_T(QSize(16, 16), QSize)));

    d->isFlightMode = isFlightMode;
    if (isFlightMode) {
        d->snackWidget->setVisible(false);
    } else {
        d->snackWidget->setVisible(this->isVisible());
    }
}

void ChunkWidget::setText(QString text) {
    ui->textLabel->setText(text);
}

void ChunkWidget::mousePressEvent(QMouseEvent *event) {
    emit showNetworkPane();
}

void ChunkWidget::watch(DevicePanel *device) {
    endWatch();

    d->watchingPanel = device;
    d->watchConnection = connect(device, &DevicePanel::updated, this, &ChunkWidget::set);
    set();
}

void ChunkWidget::endWatch() {
    disconnect(d->watchConnection);
    d->watchingPanel.clear();
}

QLabel* ChunkWidget::snackWidget() {
    return d->snackWidget;
}

void ChunkWidget::set() {
    if (!d->watchingPanel.isNull()) {
        QString finalText;
        if (d->supplementaryText == "") {
            finalText = d->watchingPanel->chunkText();
        } else {
            finalText = d->watchingPanel->chunkText() + " · " + d->supplementaryText;
        }

        setIcon(d->watchingPanel->chunkIcon());
        setText(finalText);
    }
}

void ChunkWidget::setVisible(bool visible) {
    if (d->isFlightMode) {
        d->snackWidget->setVisible(false);
    } else {
        d->snackWidget->setVisible(visible);
    }

    QWidget::setVisible(visible);
}

void ChunkWidget::setSupplementaryText(QString supplementaryText) {
    d->supplementaryText = supplementaryText;
    set();
}
