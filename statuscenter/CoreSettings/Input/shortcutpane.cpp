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
#include <QScroller>
#include "shortcutedit.h"
#include <globalkeyboard/globalkeyboardengine.h>

struct ShortcutDescriptor {
    QString humanReadableName;
    QString keyDescription;
    QString settingName;
    QString keyName;

    QList<QKeySequence> defaultSequence;

    std::function<void()> activationFunction = [=]{};
    QObject* contextObject = nullptr;
};

struct ShortcutPanePrivate {
    QGridLayout* currentSection = nullptr;
    int currentRow = 1;

    QSettings* settings;
    QString currentSectionName;
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
        addSection(tr("System"));
        addShortcut({tr("Run"), tr("Run a command"), "Run", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Run), {QKeySequence(Qt::ALT | Qt::Key_F2), QKeySequence(Qt::META | Qt::Key_R)}});
        addShortcut({tr("Suspend"), tr("Suspend the system"), "Suspend", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Suspend), {QKeySequence(Qt::Key_PowerDown)}});
        addShortcut({tr("Power Off"), tr("Power off the system, or show power options"), "PowerOff", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::PowerOff), {QKeySequence(Qt::Key_PowerOff)}});
        addShortcut({tr("Power Options"), tr("Show power options"), "PowerOptions", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::PowerOptions), {QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Delete)}});
        addShortcut({tr("Eject"), tr("Eject an optical disc"), "Eject", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Eject), {QKeySequence(Qt::Key_Eject)}});

        addSection(tr("Screen"));
        addShortcut({tr("Brightness Up"), tr("Adjust the brightness of your screen up"), "BrightnessUp", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessUp), {QKeySequence(Qt::Key_MonBrightnessUp), QKeySequence(Qt::META | Qt::Key_VolumeUp)}});
        addShortcut({tr("Brightness Down"), tr("Adjust the brightness of your screen down"), "BrightnessDown", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessDown), {QKeySequence(Qt::Key_MonBrightnessDown), QKeySequence(Qt::META | Qt::Key_VolumeDown)}});
        addShortcut({tr("Lock Screen"), tr("Lock your computer so no one else can access it"), "LockScreen", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::LockScreen), {QKeySequence(Qt::META | Qt::Key_L)}});
        addShortcut({tr("Take Screenshot"), tr("Take a photo of your screen"), "Screenshot", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::TakeScreenshot), {QKeySequence(Qt::Key_Print), QKeySequence(Qt::META | Qt::ALT | Qt::Key_P), QKeySequence(Qt::META | Qt::Key_PowerOff)}});
        addShortcut({tr("Take Screen Recording"), tr("Record a video of your screen"), "ScreenRecord", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::CaptureScreenVideo), {QKeySequence(Qt::SHIFT | Qt::Key_Print), QKeySequence(Qt::META | Qt::ALT | Qt::Key_O)}});

        addSection(tr("Audio"));
        addShortcut({tr("Volume Up"), tr("Increase the volume"), "VolumeUp", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::VolumeUp), {QKeySequence(Qt::Key_VolumeUp)}});
        addShortcut({tr("Volume Down"), tr("Decrease the volume"), "VolumeDown", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::VolumeDown), {QKeySequence(Qt::Key_VolumeDown)}});
        addShortcut({tr("Toggle Quiet Mode"), tr("Switch between Quiet Mode options"), "QuietModeToggle", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::QuietModeToggle), {QKeySequence(Qt::Key_VolumeMute)}});

        addSection(tr("Keyboard"));
        addShortcut({tr("Next Layout"), tr("Switch to the next keyboard layout"), "NextKbdLayout", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::NextKeyboardLayout), {QKeySequence(Qt::META | Qt::Key_Return)}});
        addShortcut({tr("Keyboard Brightness Up"), tr("Turn the keyboard brightness up"), "KbdBrightnessUp", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::KeyboardBrightnessUp), {QKeySequence(Qt::Key_KeyboardBrightnessUp)}});
        addShortcut({tr("Keyboard Brightness Down"), tr("Turn the keyboard brightness down"), "KbdBrightnessDown", GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::KeyboardBrightnessDown), {QKeySequence(Qt::Key_KeyboardBrightnessDown)}});
    });

    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

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

    d->currentSectionName = title;
}

void ShortcutPane::addShortcut(ShortcutDescriptor shortcut) {
    QLabel* shortcutLabel = new QLabel();
    shortcutLabel->setText(shortcut.humanReadableName);
    d->currentSection->addWidget(shortcutLabel, d->currentRow, 0);

    for (int i = 0; i < 4; i++) {
        QKeySequence defaultShortcut;
        if (shortcut.defaultSequence.count() > i) {
            defaultShortcut = shortcut.defaultSequence.at(i);
        }

        ShortcutEdit* widget = new ShortcutEdit(d->settings, shortcut.settingName, shortcut.keyName, shortcut.humanReadableName, d->currentSectionName, shortcut.keyDescription, i, defaultShortcut);
        d->currentSection->addWidget(widget, d->currentRow, i + 1);

        if (shortcut.contextObject == nullptr) {
            connect(widget, &ShortcutEdit::activated, shortcut.activationFunction);
        } else {
            connect(widget, &ShortcutEdit::activated, shortcut.contextObject, shortcut.activationFunction);
        }
    }

    d->currentRow++;
}

void ShortcutPane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
