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

#include <QIcon>
#include <the-libs_global.h>

ChunkWidget::ChunkWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChunkWidget)
{
    ui->setupUi(this);
}

ChunkWidget::~ChunkWidget()
{
    delete ui;
}

void ChunkWidget::setIcon(QIcon icon) {
    ui->iconLabel->setPixmap(icon.pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
}

void ChunkWidget::setText(QString text) {
    ui->textLabel->setText(text);
}

void ChunkWidget::mousePressEvent(QMouseEvent *event) {
    emit showNetworkPane();
}
