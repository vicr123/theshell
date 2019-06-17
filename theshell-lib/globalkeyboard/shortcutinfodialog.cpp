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
#include "shortcutinfodialog.h"
#include "ui_shortcutinfodialog.h"

#include <QTimer>
#include <QScreen>
#include <QPainter>
#include <the-libs_global.h>
#include "globalkeyboardengine.h"

struct ShortcutInfoDialogPrivate {
    QList<QWidget*> chordedAvailableKeys;
};

ShortcutInfoDialog::ShortcutInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutInfoDialog)
{
    ui->setupUi(this);
    d = new ShortcutInfoDialogPrivate;

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_ShowWithoutActivating, true);
}

ShortcutInfoDialog::~ShortcutInfoDialog()
{
    delete d;
    delete ui;
}

void ShortcutInfoDialog::showChords(QKeySequence currentKey, QList<GlobalKeyboardKey *> availableKeys, QString status) {
    ui->currentKeypressLabel->setPixmap(GlobalKeyboardEngine::getKeyShortcutImage(currentKey, this->font(), this->palette()));
    ui->statusLabel->setText(status);

    qDeleteAll(d->chordedAvailableKeys);
    d->chordedAvailableKeys.clear();
    for (int i = 0; i < availableKeys.count(); i++) {
        GlobalKeyboardKey* key = availableKeys.at(i);

        QLabel* pictogram = new QLabel();
        pictogram->setPixmap(GlobalKeyboardEngine::getKeyShortcutImage(key->key(), this->font(), this->palette()));
        ui->possibleCombinationsLayout->addWidget(pictogram, i, 0);
        d->chordedAvailableKeys.append(pictogram);

        QLabel* description = new QLabel();
        description->setText(key->name());
        description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        ui->possibleCombinationsLayout->addWidget(description, i, 1);
        d->chordedAvailableKeys.append(description);
    }

    this->show();
}

void ShortcutInfoDialog::show() {
    QRect screenGeometry = QApplication::screens().first()->geometry();
    QRect geometry;
    geometry.setSize(this->sizeHint());
    geometry.moveBottomRight(screenGeometry.bottomRight() - SC_DPI_T(QPoint(10, 10), QPoint));
    this->setGeometry(geometry);
    this->setFixedSize(this->sizeHint());
    QDialog::show();

    QTimer::singleShot(0, [=] {
        QRect screenGeometry = QApplication::screens().first()->geometry();
        QRect geometry;
        geometry.setSize(this->sizeHint());
        geometry.moveBottomRight(screenGeometry.bottomRight() - SC_DPI_T(QPoint(10, 10), QPoint));
        this->setGeometry(geometry);
        this->setFixedSize(this->sizeHint());
    });
}

void ShortcutInfoDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, 0, this->width(), 0);
    painter.drawLine(0, 0, 0, this->height());
    painter.drawLine(0, this->height() - 1, this->width() - 1, this->height() - 1);
    painter.drawLine(this->width() - 1, 0, this->width() - 1, this->height() - 1);
}
