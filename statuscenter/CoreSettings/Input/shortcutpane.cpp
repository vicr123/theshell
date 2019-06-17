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
#include "shortcutpane.h"
#include "ui_shortcutpane.h"

#include <functional>
#include <QMessageBox>
#include <QTimer>
#include "shortcutedit.h"
#include <globalkeyboard/globalkeyboardengine.h>

struct ShortcutDescriptor {
    QString name;
    QString settingName;
    QString keyName;
    QString keyDescription;

    QList<QKeySequence> defaultSequence;

    std::function<void()> activationFunction = [=]{};
    QObject* contextObject = nullptr;
};

struct ShortcutPanePrivate {
    QGridLayout* currentSection = nullptr;
    int currentRow = 1;

    QSettings* settings;
};

ShortcutPane::ShortcutPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShortcutPane)
{
    ui->setupUi(this);

    d = new ShortcutPanePrivate();
    d->settings = new QSettings("theSuite", "theShell-shortcuts");
    d->settings->beginGroup("shortcuts");

    QTimer::singleShot(0, [=] {
        //Do this once everything is ready so that we know they're all listening for key shortcuts
        addSection(tr("Screen"));
        addShortcut({tr("Brightness Up"), "BrightnessUp", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessUp), tr("Adjust the brightness of your screen up"), {QKeySequence(Qt::Key_MonBrightnessUp), QKeySequence(Qt::META | Qt::Key_VolumeUp)}});
        addShortcut({tr("Brightness Down"), "BrightnessDown", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessDown), tr("Adjust the brightness of your screen down"), {QKeySequence(Qt::Key_MonBrightnessDown), QKeySequence(Qt::META | Qt::Key_VolumeDown)}});
    });}

ShortcutPane::~ShortcutPane()
{
    d->settings->deleteLater();
    delete d;
    delete ui;
}

void ShortcutPane::addSection(QString title) {
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFixedHeight(1);
    ui->shortcutsLayout->addWidget(line);

    d->currentSection = new QGridLayout();
    d->currentSection->setContentsMargins(9, 9, 9, 9);
    d->currentSection->setSpacing(6);
    ui->shortcutsLayout->addLayout(d->currentSection);
    d->currentRow = 1;

    QLabel* titleLabel = new QLabel();
    QFont fnt = titleLabel->font();
    fnt.setBold(true);
    titleLabel->setFont(fnt);
    titleLabel->setText(title.toUpper());
    d->currentSection->addWidget(titleLabel, 0, 0);
}

void ShortcutPane::addShortcut(ShortcutDescriptor shortcut) {
    QLabel* shortcutLabel = new QLabel();
    shortcutLabel->setText(shortcut.name);
    d->currentSection->addWidget(shortcutLabel, d->currentRow, 0);

    for (int i = 0; i < 4; i++) {
        QKeySequence defaultShortcut;
        if (shortcut.defaultSequence.count() > i) {
            defaultShortcut = shortcut.defaultSequence.at(i);
        }

        ShortcutEdit* widget = new ShortcutEdit(d->settings, shortcut.settingName, shortcut.keyName, i, defaultShortcut);
        d->currentSection->addWidget(widget, d->currentRow, i + 1);

        if (shortcut.contextObject == nullptr) {
            connect(widget, &ShortcutEdit::activated, shortcut.activationFunction);
        } else {
            connect(widget, &ShortcutEdit::activated, shortcut.contextObject, shortcut.activationFunction);
        }
    }

    d->currentRow++;
}
