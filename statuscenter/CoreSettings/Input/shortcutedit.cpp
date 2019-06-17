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
#include "shortcutedit.h"

#include <QPainter>
#include <QKeyEvent>
#include <QMenu>
#include <the-libs_global.h>
#include <globalkeyboard/globalkeyboardengine.h>

struct ShortcutEditPrivate {
    QString setting;
    QString keyName;
    int index;

    QSettings* settings;

    QKeySequence currentSequence;
    bool editing = false;

    int currentKey = 0;
    Qt::Key editingKeys[4];
    Qt::Key previousKey;

    GlobalKeyboardKey* currentCapture = nullptr;

    QString humanReadableName;
    QString section;
    QString description;
};

ShortcutEdit::ShortcutEdit(QSettings* settings, QString setting, QString keyName, QString humanReadableName, QString section, QString description, int index, QKeySequence defaultShortcut, QWidget *parent) : QWidget(parent)
{
    d = new ShortcutEditPrivate();
    d->settings = settings;

    d->setting = setting;
    d->index = index;
    d->keyName = keyName;

    d->humanReadableName = humanReadableName;
    d->section = section;
    d->description = description;

    d->currentSequence = QKeySequence(d->settings->value(d->setting + "-" + QString::number(d->index), defaultShortcut.toString()).toString());

    this->setFocusPolicy(Qt::StrongFocus);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &ShortcutEdit::customContextMenuRequested, [=](QPoint pos) {
        QMenu* menu = new QMenu();
        menu->addSection(tr("For Shortcut %1").arg(d->currentSequence.toString()));
        menu->addAction(tr("Reset"), [=] {
            d->currentSequence = defaultShortcut;
            editingDone();
        });
        menu->addAction(tr("Clear"), [=] {
            d->currentSequence = QKeySequence();
            editingDone();
        });
        menu->exec(this->mapToGlobal(pos));
    });

    editingDone();
}

ShortcutEdit::~ShortcutEdit() {
    delete d;
}

void ShortcutEdit::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setFont(this->font());
    painter.setPen(this->palette().color(QPalette::WindowText));

    int currentX = SC_DPI(4);
    /*
    QString sequence = d->currentSequence.toString();
    QStringList chordParts = sequence.split(", ", QString::SkipEmptyParts);
    if (chordParts.count() == 0) {
        QRect textRect;
        textRect.setWidth(this->fontMetrics().width(tr("No Shortcut")) + 1);
        textRect.setHeight(this->fontMetrics().height());
        textRect.moveLeft(currentX);
        textRect.moveTop(this->height() / 2 - textRect.height() / 2);
        painter.drawText(textRect, tr("No Shortcut"));
        currentX = textRect.right() + SC_DPI(4);
    } else {
        for (int i = 0; i < chordParts.count(); i++) {
            QStringList keys = chordParts.at(i).split("+", QString::SkipEmptyParts);

            for (QString key : keys) {
                QPixmap icon = getKeyIcon(key);
                painter.drawPixmap(currentX, SC_DPI(4), icon);
                currentX += icon.width() + SC_DPI(4);
            }

            if (chordParts.count() > i + 1) {
                QRect textRect;
                textRect.setWidth(this->fontMetrics().width(",") + 1);
                textRect.setHeight(this->fontMetrics().height());
                textRect.moveLeft(currentX);
                textRect.moveTop(this->height() / 2 - textRect.height() / 2);
                painter.drawText(textRect, ",");
                currentX = textRect.right() + SC_DPI(4);
            }
        }
    }*/

    QPixmap shortcutPixmap = GlobalKeyboardEngine::getKeyShortcutImage(d->currentSequence, this->font(), this->palette());
    QRect pxRect;
    pxRect.setSize(shortcutPixmap.size());
    pxRect.moveLeft(SC_DPI(4));
    pxRect.moveTop(this->height() / 2 - pxRect.height() / 2);
    painter.drawPixmap(pxRect, shortcutPixmap);
    currentX += shortcutPixmap.width() + SC_DPI(4);

    if (d->editing) {
        painter.setPen(this->palette().color(QPalette::Disabled, QPalette::WindowText));

        QRect textRect;
        textRect.setWidth(this->fontMetrics().width(tr("Type Shortcut")) + 1);
        textRect.setHeight(this->fontMetrics().height());
        textRect.moveLeft(currentX);
        textRect.moveTop(this->height() / 2 - textRect.height() / 2);
        painter.drawText(textRect, tr("Type Shortcut"));
        currentX = textRect.right() + SC_DPI(4);
    }
}

QSize ShortcutEdit::sizeHint() const {
    return QSize(0, qMax(SC_DPI(24), this->fontMetrics().height() + SC_DPI(8)));
}

void ShortcutEdit::focusInEvent(QFocusEvent *event) {
    d->editing = true;
    d->currentKey = 0;
    GlobalKeyboardEngine::pauseListening();
    this->update();
}

void ShortcutEdit::focusOutEvent(QFocusEvent *event) {
    GlobalKeyboardEngine::startListening();
    editingDone();
}

void ShortcutEdit::keyPressEvent(QKeyEvent *event) {
    if (d->editing) {
        event->accept();
        if (event->isAutoRepeat()) return;

        if (d->currentKey == 0) {
            d->editingKeys[0] = d->editingKeys[1] = d->editingKeys[2] = d->editingKeys[3] = static_cast<Qt::Key>(0);
        }

        int key = event->key();
        if (isModifierKey(static_cast<Qt::Key>(key))) {
            d->editingKeys[d->currentKey] = static_cast<Qt::Key>(static_cast<int>(event->modifiers()));
        } else {
            d->editingKeys[d->currentKey] = static_cast<Qt::Key>(key | event->modifiers());
        }
        d->currentSequence = QKeySequence(d->editingKeys[0], d->editingKeys[1], d->editingKeys[2], d->editingKeys[3]);

        this->update();

        if (isModifierKey(static_cast<Qt::Key>(key))) return;
        d->previousKey = static_cast<Qt::Key>(key);
        d->currentKey++;

        //Disable editing once we get to the 4th key
        if (d->currentKey == 4) editingDone();
    }
}

void ShortcutEdit::keyReleaseEvent(QKeyEvent *event) {
    if (isModifierKey(d->editingKeys[d->currentKey])) d->editingKeys[d->currentKey] = static_cast<Qt::Key>(0);
    d->currentSequence = QKeySequence(d->editingKeys[0], d->editingKeys[1], d->editingKeys[2], d->editingKeys[3]);
    this->update();
}

bool ShortcutEdit::isModifierKey(Qt::Key key) {
    return (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Meta || key == Qt::Key_Alt || key == Qt::Key_unknown);
}

QPixmap ShortcutEdit::getKeyIcon(QString key) {
    if (key == "Meta") key = "Super";

    QPixmap squarePx(SC_DPI_T(QSize(16, 16), QSize));
    squarePx.fill(Qt::transparent);

    QPainter sqPainter(&squarePx);
    sqPainter.setRenderHint(QPainter::Antialiasing);
    sqPainter.setPen(Qt::transparent);
    sqPainter.setBrush(this->palette().color(QPalette::WindowText));
    sqPainter.drawRoundedRect(QRect(QPoint(0, 0), squarePx.size()), 50, 50, Qt::RelativeSize);

    QRect squareIconRect;
    squareIconRect.setWidth(12 * theLibsGlobal::getDPIScaling());
    squareIconRect.setHeight(12 * theLibsGlobal::getDPIScaling());
    squareIconRect.moveCenter(QPoint(8, 8) * theLibsGlobal::getDPIScaling());

    /*if (key == "Left") {
        QImage image = QIcon::fromTheme("go-previous").pixmap(squareIconRect.size()).toImage();
        tintImage(image, this->palette().color(QPalette::Window));
        sqPainter.drawImage(squareIconRect, image);
        return squarePx;
    } else if (key == "Right") {
        QImage image = QIcon::fromTheme("go-next").pixmap(squareIconRect.size()).toImage();
        tintImage(image, this->palette().color(QPalette::Window));
        sqPainter.drawImage(squareIconRect, image);
        return squarePx;
    } else {*/
        QFont font = this->font();
        QFontMetrics fontMetrics(font);

        //font.setPointSizeF(floor(8));
        while (QFontMetrics(font).height() > 14 * theLibsGlobal::getDPIScaling()) {
            font.setPointSizeF(font.pointSizeF() - 0.5);
        }

        QSize pixmapSize;
        pixmapSize.setHeight(16 * theLibsGlobal::getDPIScaling());
        pixmapSize.setWidth(qMax(fontMetrics.width(key) + 6 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling()));

        QPixmap px(pixmapSize);
        px.fill(Qt::transparent);

        QPainter painter(&px);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::transparent);
        painter.setBrush(this->palette().color(QPalette::WindowText));
        painter.drawRoundedRect(QRect(QPoint(0, 0), px.size()), 4 * theLibsGlobal::getDPIScaling(), 4 * theLibsGlobal::getDPIScaling());

        painter.setFont(font);
        painter.setPen(this->palette().color(QPalette::Window));

        QRect textRect;
        textRect.setHeight(fontMetrics.height());
        textRect.setWidth(fontMetrics.width(key));
        textRect.moveCenter(QPoint(pixmapSize.width() / 2, pixmapSize.height() / 2));

        painter.drawText(textRect, Qt::AlignCenter, key);

        painter.end();
        return px;
    //}
}

void ShortcutEdit::editingDone() {
    d->editing = false;
    this->update();

    d->settings->setValue(d->setting + "-" + QString::number(d->index), d->currentSequence.toString());

    if (d->currentCapture != nullptr) {
        d->currentCapture->deregister();
    }
    d->currentCapture = GlobalKeyboardEngine::registerKey(d->currentSequence, d->keyName, d->section, d->humanReadableName, d->description);
    if (d->currentCapture != nullptr) connect(d->currentCapture, &GlobalKeyboardKey::shortcutActivated, this, &ShortcutEdit::activated);
}
