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

#include <QDBusInterface>
#include <math.h>
#include <globalkeyboard/globalkeyboardengine.h>
#include <soundengine.h>
#include <tsystemsound.h>
#include <QX11Info>
#include <QScreen>

struct HotkeyHudPrivate {
    HotkeyHud* instance = nullptr;
    bool isShowing = false;
    int value;
    QTimer* timeout = nullptr;

    QColor highlightCol;
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
        } else if (name ==  GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::KeyboardBrightnessUp)) {
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
        } else if (name ==  GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::KeyboardBrightnessDown)) {
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

    QColor highlightCol = d->highlightCol;
    highlightCol.setAlpha(127);
    //QColor backgroundCol = pal.color(QPalette::Window);
    //int average = (backgroundCol.red() + backgroundCol.green() + backgroundCol.blue()) / 3;
    int value = d->value;
    while (value > 0) {
        /*if (average < 127) { //Dark color
            highlightCol = highlightCol.lighter(150);
        } else {
            highlightCol = highlightCol.darker(150);
        }*/
        painter.setBrush(highlightCol);
        painter.setPen(Qt::transparent);
        painter.drawRect(0, 0, qRound(value / 100.0 * this->width()), this->height() - 1);
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
        QRect screenGeometry = QApplication::screens().first()->geometry();
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
    HotkeyHud::show({
        {"icon", icon},
        {"control", control},
        {"value", value}
    });
}

void HotkeyHud::show(QIcon icon, QString control, QString explanation, int timeout) {
    HotkeyHud::show({
        {"icon", icon},
        {"control", control},
        {"explanation", explanation},
        {"timeout", timeout}
    });
}

void HotkeyHud::show(QVariantMap options) {
    makeInstance(); //Ensure instance exists

    if (options.contains("icon")) {
        d->instance->ui->icon->setPixmap(options.value("icon").value<QIcon>().pixmap(32));
    }

    if (options.contains("control")) {
        d->instance->ui->control->setText(options.value("control").toString());
    }

    if (options.contains("value")) {
        d->value = options.value("value").toInt();
        d->instance->ui->value->setText(QString::number(d->value) + "%");
        d->instance->ui->value->setVisible(true);
    } else {
        d->value = 0;
        d->instance->ui->value->setVisible(false);
    }

    if (options.contains("highlightCol")) {
        d->highlightCol = options.value("highlightCol").value<QColor>();
    } else {
        QColor col = d->instance->palette().color(QPalette::WindowText);
        int average = (col.red() + col.green() + col.blue()) / 3;
        if (average < 127) { //Dark color
            col = col.lighter(150);
        } else {
            col = col.darker(150);
        }
        d->highlightCol = col;
    }

    if (options.contains("sound")) {
        tSystemSound::play(options.value("sound").toString());
    }

    d->instance->ui->explanation->setText(options.value("explanation", "").toString());
    d->instance->show(options.value("timeout", 1500).toInt());
    d->instance->repaint();
}

void HotkeyHud::close() {
    QRect screenGeometry = QApplication::screens().first()->geometry();

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

HotkeyHud* HotkeyHud::instance() {
    makeInstance();
    return d->instance;
}
