/****************************************
 *
 *   theShell - Desktop Environment
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

#include "hotkeyhud.h"
#include "ui_hotkeyhud.h"

#include <math.h>
#include <globalkeyboard/globalkeyboardengine.h>
#include "audiomanager.h"
#include <soundengine.h>
#include "mainwindow.h"

extern AudioManager* AudioMan;
extern MainWindow* MainWin;

struct HotkeyHudPrivate {
    HotkeyHud* instance = nullptr;
    bool isShowing = false;
    int value;
    QTimer* timeout = nullptr;
};

HotkeyHudPrivate* HotkeyHud::d = new HotkeyHudPrivate();

HotkeyHud::HotkeyHud(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HotkeyHud)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_ShowWithoutActivating, true);

    connect(GlobalKeyboardEngine::instance(), &GlobalKeyboardEngine::keyShortcutRegistered, this, [=](QString name, GlobalKeyboardKey* key) {
        if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessUp)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                //Get Current Brightness
                QProcess backlight;
                backlight.start("xbacklight -get");
                backlight.waitForFinished();
                float currentBrightness = ceil(QString(backlight.readAll()).toFloat());

                currentBrightness = currentBrightness + 10;
                if (currentBrightness > 100) currentBrightness = 100;

                QProcess* backlightAdj = new QProcess(this);
                backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                HotkeyHud::show(QIcon::fromTheme("video-display"), tr("Brightness"), (int) currentBrightness);
            });
        } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::BrightnessDown)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                //Get Current Brightness
                QProcess backlight;
                backlight.start("xbacklight -get");
                backlight.waitForFinished();
                float currentBrightness = ceil(QString(backlight.readAll()).toFloat());

                currentBrightness = currentBrightness - 10;
                if (currentBrightness < 0) currentBrightness = 0;

                QProcess* backlightAdj = new QProcess(this);
                backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                HotkeyHud::show(QIcon::fromTheme("video-display"), tr("Brightness"), (int) currentBrightness);
            });
        } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::VolumeUp)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                //Increase volume
                if (AudioMan->QuietMode() == AudioManager::mute) {
                    HotkeyHud::show(QIcon::fromTheme("audio-volume-muted"), tr("Volume"), tr("Quiet Mode is set to Mute."));
                } else {
                    int volume = AudioMan->MasterVolume();
                    volume += 5;
                    if (volume < 0) volume = 0;
                    AudioMan->changeVolume(5);

                    //Play the audio change sound
                    SoundEngine::play(SoundEngine::Volume);

                    HotkeyHud::show(QIcon::fromTheme("audio-volume-high"), tr("Volume"), volume);
                }
            });
        } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::VolumeDown)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                //Decrease volume
                if (AudioMan->QuietMode() == AudioManager::mute) {
                    HotkeyHud::show(QIcon::fromTheme("audio-volume-muted"), tr("Volume"), tr("Quiet Mode is set to Mute."));
                } else {
                    int volume = AudioMan->MasterVolume();
                    volume -= 5;
                    if (volume < 0) volume = 0;
                    AudioMan->changeVolume(-5);

                    //Play the audio change sound
                    SoundEngine::play(SoundEngine::Volume);

                    HotkeyHud::show(QIcon::fromTheme("audio-volume-high"), tr("Volume"), volume);
                }
            });
        } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::QuietModeToggle)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                switch (AudioMan->QuietMode()) {
                    case AudioManager::none:
                        AudioMan->setQuietMode(AudioManager::critical);
                        HotkeyHud::show(QIcon::fromTheme("quiet-mode-critical-only"), tr("Critical Only"), AudioMan->getCurrentQuietModeDescription(), 5000);
                        break;
                    case AudioManager::critical:
                        AudioMan->setQuietMode(AudioManager::notifications);
                        HotkeyHud::show(QIcon::fromTheme("quiet-mode"), tr("No Notifications"), AudioMan->getCurrentQuietModeDescription(), 5000);
                        break;
                    case AudioManager::notifications:
                        AudioMan->setQuietMode(AudioManager::mute);
                        HotkeyHud::show(QIcon::fromTheme("audio-volume-muted"), tr("Mute"), AudioMan->getCurrentQuietModeDescription(), 5000);
                        break;
                    case AudioManager::mute:
                        AudioMan->setQuietMode(AudioManager::none);
                        HotkeyHud::show(QIcon::fromTheme("audio-volume-high"), tr("Sound"), AudioMan->getCurrentQuietModeDescription(), 5000);
                        break;
                }
            });
        } else if (name == GlobalKeyboardEngine::NextKeyboardLayout) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                QString newKeyboardLayout = MainWin->getInfoPane()->setNextKeyboardLayout();
                HotkeyHud::show(QIcon::fromTheme("input-keyboard"), tr("Keyboard Layout"), tr("Keyboard Layout set to %1").arg(newKeyboardLayout), 5000);
            });
        } else if (name == GlobalKeyboardEngine::KeyboardBrightnessUp) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                int kbdBrightness = -1, maxKbdBrightness = -1;
                QDBusInterface keyboardInterface("org.freedesktop.UPower", "/org/freedesktop/UPower/KbdBacklight", "org.freedesktop.UPower.KbdBacklight", QDBusConnection::systemBus());
                if (keyboardInterface.isValid()) {
                    kbdBrightness = keyboardInterface.call("GetBrightness").arguments().first().toInt();
                    maxKbdBrightness = keyboardInterface.call("GetMaxBrightness").arguments().first().toInt();
                }

                kbdBrightness += (((float) maxKbdBrightness / 100) * 5);
                if (kbdBrightness > maxKbdBrightness) kbdBrightness = maxKbdBrightness;
                keyboardInterface.call("SetBrightness", kbdBrightness);

                HotkeyHud::show(QIcon::fromTheme("keyboard-brightness"), tr("Keyboard Brightness"), ((float) kbdBrightness / (float) maxKbdBrightness) * 100);
            });
        } else if (name == GlobalKeyboardEngine::KeyboardBrightnessDown) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                int kbdBrightness = -1, maxKbdBrightness = -1;
                QDBusInterface keyboardInterface("org.freedesktop.UPower", "/org/freedesktop/UPower/KbdBacklight", "org.freedesktop.UPower.KbdBacklight", QDBusConnection::systemBus());
                if (keyboardInterface.isValid()) {
                    kbdBrightness = keyboardInterface.call("GetBrightness").arguments().first().toInt();
                    maxKbdBrightness = keyboardInterface.call("GetMaxBrightness").arguments().first().toInt();
                }

                kbdBrightness -= (((float) maxKbdBrightness / 100) * 5);
                if (kbdBrightness < 0) kbdBrightness = 0;
                keyboardInterface.call("SetBrightness", kbdBrightness);

                HotkeyHud::show(QIcon::fromTheme("keyboard-brightness"), tr("Keyboard Brightness"), ((float) kbdBrightness / (float) maxKbdBrightness) * 100);
            });
        } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Eject)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                QProcess* eject = new QProcess(this);
                eject->start("eject");
                connect(eject, SIGNAL(finished(int)), eject, SLOT(deleteLater()));

                HotkeyHud::show(QIcon::fromTheme("media-eject"), tr("Eject"), tr("Attempting to eject disc..."));
            });
        }
    });
}

HotkeyHud::~HotkeyHud()
{
    delete ui;
}

void HotkeyHud::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void HotkeyHud::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void HotkeyHud::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);

    QPalette pal = this->palette();

    QColor highlightCol = pal.color(QPalette::Window);
    int average = (highlightCol.red() + highlightCol.green() + highlightCol.blue()) / 3;
    int value = d->value;
    while (value > 0) {
        if (average < 127) { //Dark color
            highlightCol = highlightCol.light(150);
        } else {
            highlightCol = highlightCol.dark(150);
        }
        painter.setBrush(highlightCol);
        painter.setPen(Qt::transparent);
        painter.drawRect(0, 0, ((float) value / (float) 100) * this->width(), this->height() - 1);
        value -= 100;
    }

    event->accept();
}

void HotkeyHud::show(int timeout) {
    makeInstance(); //Ensure instance exists

    d->instance->setFixedHeight(SC_DPI(d->instance->sizeHint().height()));

    Atom atoms[2];
    atoms[0] = XInternAtom(QX11Info::display(), "_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY", False);
    atoms[1] = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    int retval = XChangeProperty(QX11Info::display(), d->instance->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &atoms, 2); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    retval = XChangeProperty(QX11Info::display(), d->instance->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    d->instance->QDialog::show();

    if (!d->isShowing) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        d->instance->setGeometry(screenGeometry.x(), screenGeometry.y() - d->instance->height(), screenGeometry.width(), d->instance->height());

        tPropertyAnimation *anim = new tPropertyAnimation(d->instance, "geometry");
        anim->setStartValue(d->instance->geometry());
        anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), d->instance->height()));
        anim->setDuration(100);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();
        connect(anim, &tPropertyAnimation::finished, d->instance, [=](){
            d->instance->repaint();
        });
    }

    if (d->timeout == nullptr) {
        d->timeout = new QTimer();
        d->timeout->setSingleShot(true);
        d->timeout->setInterval(timeout);
        connect(d->timeout, &QTimer::timeout, [=] {
            d->timeout->deleteLater();
            d->timeout = nullptr;
            d->instance->close();
        });

    }
    d->timeout->start();

    d->isShowing = true;
}

void HotkeyHud::show(QIcon icon, QString control, int value) {
    makeInstance(); //Ensure instance exists

    d->instance->ui->icon->setPixmap(icon.pixmap(32));
    d->instance->ui->control->setText(control);
    d->instance->ui->value->setText(QString::number(value) + "%");
    d->instance->ui->explanation->setText("");
    d->instance->ui->value->setVisible(true);
    d->value = value;
    d->instance->show();
    d->instance->repaint();
}

void HotkeyHud::show(QIcon icon, QString control, QString explanation, int timeout) {
    makeInstance(); //Ensure instance exists

    d->instance->ui->icon->setPixmap(icon.pixmap(32));
    d->instance->ui->control->setText(control);
    d->instance->ui->explanation->setText(explanation);
    d->instance->ui->value->setVisible(false);
    d->instance->ui->explanation->setVisible(true);
    d->value = 0;
    d->instance->show(timeout);
    d->instance->repaint();
}

void HotkeyHud::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    tPropertyAnimation *anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tPropertyAnimation::finished, [=]() {
        d->instance->QDialog::close();
        d->isShowing = false;
    });
    anim->start();
}

void HotkeyHud::makeInstance() {
    if (d->instance == nullptr) d->instance = new HotkeyHud();
}
