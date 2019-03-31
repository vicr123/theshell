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
#include "mousepane.h"
#include "ui_mousepane.h"

#include "mousepanetester.h"

#include <math.h>
#include <QSettings>
#include <QX11Info>
#include <QDebug>
#include <QMouseEvent>

#include <libinput-properties.h>
#include <synaptics-properties.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XInput.h>

#undef Bool

struct MousePanePrivate {
    enum WriteFor {
        Mice = 1,
        Touchpads = 2,
        AllDevices = Mice | Touchpads
    };

    QSettings settings;

    int numButtons = 0;
    int middleButton = -1;

    bool useLibInput = false;

    void writeXiSetting(const char* atom, QVariantList value, WriteFor writeFor = AllDevices) {
        int devices;
        XDeviceInfo* info = XListInputDevices(QX11Info::display(), &devices);
        if (info == nullptr) return;

        Atom mouseAtom = XInternAtom(QX11Info::display(), XI_MOUSE, True);
        Atom touchpadAtom = XInternAtom(QX11Info::display(), XI_TOUCHPAD, True);
        Atom trackballAtom = XInternAtom(QX11Info::display(), XI_TRACKBALL, True);
        for (int i = 0; i < devices; i++) {
            XDeviceInfo* d = info + i;
            if ((d->type == mouseAtom && writeFor & Mice)
             || (d->type == touchpadAtom && writeFor & Touchpads)
             || (d->type == trackballAtom && writeFor & Mice)) {
                Atom valAtom = XInternAtom(QX11Info::display(), atom, True);
                Atom type;
                if (value.first().type() == QVariant::Bool || value.first().type() == QVariant::Int || value.first().type() == QVariant::Char) {
                    type = XA_INTEGER;
                } else if (value.first().type() == QVariant::Double) {
                    type = XInternAtom(QX11Info::display(), "FLOAT", False);
                } else {
                    return;
                }

                Atom typeReturn;
                int formatReturn;
                unsigned long itemCount;
                unsigned long bytesAfter;
                unsigned char* data = nullptr;
                Status s = XIGetProperty(QX11Info::display(), d->id, valAtom, 0, 32, False, type, &typeReturn, &formatReturn, &itemCount, &bytesAfter, &data);
                if (s != Success) continue;

                if (typeReturn != type || data == nullptr || itemCount != value.count()) {
                    if (data != nullptr) {
                        XFree(data);
                    }
                    continue;
                }

                if (value.first().type() == QVariant::Bool && formatReturn != 8) {
                    XFree(data);
                    continue;
                } else if (value.first().type() == QVariant::Double && formatReturn != 32) {
                    XFree(data);
                    continue;
                } else if (value.first().type() == QVariant::Char && formatReturn != 8) {
                    XFree(data);
                    continue;
                } else if (value.first().type() == QVariant::Int && formatReturn != 32) {
                    XFree(data);
                    continue;
                }

                if (value.first().type() == QVariant::Bool) {
                    unsigned char v[64];
                    for (int i = 0; i < value.count(); i++) {
                        v[i] = value.at(i).toBool() ? 1 : 0;
                    }
                    XIChangeProperty(QX11Info::display(), d->id, valAtom, type, 8, XIPropModeReplace, v, value.count());
                } else if (value.first().type() == QVariant::Double) {
                    float v[64];
                    for (int i = 0; i < value.count(); i++) {
                        v[i] = static_cast<float>(value.at(i).toDouble());
                    }
                    XIChangeProperty(QX11Info::display(), d->id, valAtom, type, 32, XIPropModeReplace, reinterpret_cast<unsigned char*>(v), value.count());
                } else if (value.first().type() == QVariant::Char) {
                    unsigned char v[64];
                    for (int i = 0; i < value.count(); i++) {
                        v[i] = value.at(i).toChar().toLatin1();
                    }
                    XIChangeProperty(QX11Info::display(), d->id, valAtom, type, 8, XIPropModeReplace, v, value.count());
                } else if (value.first().type() == QVariant::Int) {
                    int v[64];
                    for (int i = 0; i < value.count(); i++) {
                        v[i] = value.at(i).toInt();
                    }
                    XIChangeProperty(QX11Info::display(), d->id, valAtom, type, 32, XIPropModeReplace, reinterpret_cast<unsigned char*>(v), value.count());
                }

                XFree(data);
            }
        }
        XFreeDeviceList(info);
    }
};

MousePane::MousePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MousePane)
{
    ui->setupUi(this);
    d = new MousePanePrivate;

    if (d->settings.value("mouse/primaryIsLeft", true).toBool()) {
        ui->leftPrimaryButton->setChecked(true);
    } else {
        ui->rightPrimaryButton->setChecked(true);
    }
    ui->speedSlider->setValue(d->settings.value("mouse/speed", 100).toInt());
    ui->tapToClick->setChecked(d->settings.value("mouse/tapToClick", true).toBool());
    ui->naturalMouseScrolling->setChecked(d->settings.value("mouse/naturalScroll", false).toBool());
    ui->naturalTouchpadScrolling->setChecked(d->settings.value("mouse/naturalTouchpadScroll", false).toBool());

    ui->testAreaWidget->setFixedWidth(300 * theLibsGlobal::getDPIScaling());
    for (int i = 0; i < 30; i++) {
        MousePaneTester* tester = new MousePaneTester();
        tester->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        ui->mouseTestLayout->addWidget(tester);
    }
    ui->testClickButton->installEventFilter(this);

    if (XInternAtom(QX11Info::display(), LIBINPUT_PROP_ACCEL, True)) {
        //We have libinput
        qDebug() << "Using libinput";
        d->useLibInput = true;
    } else {
        unsigned char pointerMapping[256];
        d->numButtons = XGetPointerMapping(QX11Info::display(), pointerMapping, 256);
        if (d->numButtons >= 3) {
            d->middleButton = pointerMapping[1];
        }
    }

    applySettings();
}

MousePane::~MousePane()
{
    delete d;
    delete ui;
}

void MousePane::applySettings() {
    if (d->useLibInput) {
        d->writeXiSetting(LIBINPUT_PROP_LEFT_HANDED, {!d->settings.value("mouse/primaryIsLeft", true).toBool()});
        d->writeXiSetting(LIBINPUT_PROP_ACCEL, {(d->settings.value("mouse/speed", 100).toInt() / 50.0) - 1});

        float highSpeed = d->settings.value("mouse/speed", 100).toInt() / 100.0 * 5;
        d->writeXiSetting(SYNAPTICS_PROP_SPEED, {0.0, highSpeed, 0.1, 0});

        if (d->settings.value("mouse/tapToClick", true).toBool()) {
            d->writeXiSetting(LIBINPUT_PROP_TAP, {true}, MousePanePrivate::Touchpads);
            d->writeXiSetting(SYNAPTICS_PROP_TAP_ACTION, {QChar(0), QChar(0), QChar(0), QChar(0), QChar(1), QChar(0), QChar(0)}, MousePanePrivate::Touchpads);
        } else {
            d->writeXiSetting(LIBINPUT_PROP_TAP, {false}, MousePanePrivate::Touchpads);
            d->writeXiSetting(SYNAPTICS_PROP_TAP_ACTION, {QChar(0), QChar(0), QChar(0), QChar(0), QChar(0), QChar(0), QChar(0)}, MousePanePrivate::Touchpads);
        }

        d->writeXiSetting(LIBINPUT_PROP_NATURAL_SCROLL, {d->settings.value("mouse/naturalScroll", false).toBool()}, MousePanePrivate::Mice);

        if (d->settings.value("mouse/naturalTouchpadScroll", false).toBool()) {
            d->writeXiSetting(LIBINPUT_PROP_NATURAL_SCROLL, {true}, MousePanePrivate::Touchpads);
            d->writeXiSetting(SYNAPTICS_PROP_SCROLL_DISTANCE, {-200, -200}, MousePanePrivate::Touchpads);
        } else {
            d->writeXiSetting(LIBINPUT_PROP_NATURAL_SCROLL, {false}, MousePanePrivate::Touchpads);
            d->writeXiSetting(SYNAPTICS_PROP_SCROLL_DISTANCE, {200, 200}, MousePanePrivate::Touchpads);
        }
    } else {
        unsigned char pointerMapping[256];
        unsigned char newPointerMapping[256];
        XGetPointerMapping(QX11Info::display(), pointerMapping, 256);
        memcpy(pointerMapping, newPointerMapping, 256 * sizeof(unsigned char));

        bool primaryIsLeft = d->settings.value("mouse/primaryIsLeft", true).toBool();
        if (d->numButtons == 1) {
            newPointerMapping[0] = 1; //Only button is always the primary button
        } else if (d->numButtons == 2) {
            if (primaryIsLeft) {
                newPointerMapping[0] = 1;
                newPointerMapping[1] = 3;
            } else {
                newPointerMapping[0] = 3;
                newPointerMapping[1] = 1;
            }
        } else {
            if (primaryIsLeft) {
                newPointerMapping[0] = 1;
                newPointerMapping[1] = d->middleButton;
                newPointerMapping[2] = 3;
            } else {
                newPointerMapping[0] = 3;
                newPointerMapping[1] = d->middleButton;
                newPointerMapping[2] = 1;
            }
        }

        //Change the pointer mapping only if we need to
        for (int i = 0; i < d->numButtons; i++) {
            if (pointerMapping[i] != newPointerMapping[i]) {
                //Repeatedly attempt to set the pointer mapping until the pointer is no longer busy
                while (XSetPointerMapping(QX11Info::display(), newPointerMapping, d->numButtons) == MappingBusy) {}
                i = d->numButtons;
            }
        }

        double accelValue = round(pow(1.096478, d->settings.value("mouse/speed", 100).toInt()));
        XChangePointerControl(QX11Info::display(), True, False, int(accelValue), 100, 5);

        XFlush(QX11Info::display());
    }
}

void MousePane::on_leftPrimaryButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("mouse/primaryIsLeft", true);
        applySettings();
    }
}

void MousePane::on_rightPrimaryButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("mouse/primaryIsLeft", false);
        applySettings();
    }
}

void MousePane::on_speedSlider_valueChanged(int value)
{
    d->settings.setValue("mouse/speed", value);
    //Apply the settings when the slider is released
}

void MousePane::on_speedSlider_sliderReleased()
{
    applySettings();
}

void MousePane::on_tapToClick_toggled(bool checked)
{
    d->settings.setValue("mouse/tapToClick", checked);
    applySettings();
}

void MousePane::on_naturalMouseScrolling_toggled(bool checked)
{
    d->settings.setValue("mouse/naturalScroll", checked);
    applySettings();
}

void MousePane::on_naturalTouchpadScrolling_toggled(bool checked)
{
    d->settings.setValue("mouse/naturalTouchpadScroll", checked);
    applySettings();
}

bool MousePane::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->testClickButton) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* e = static_cast<QMouseEvent*>(event);
            QString button;

            switch (e->button()) {
                case Qt::LeftButton:
                    button = tr("Primary Button");
                    break;
                case Qt::MiddleButton:
                    button = tr("Middle Button");
                    break;
                case Qt::RightButton:
                    button = tr("Secondary Button");
                    break;
                case Qt::ExtraButton1:
                    button = tr("Button %1").arg("1");
                    break;
                case Qt::ExtraButton2:
                    button = tr("Button %1").arg("2");
                    break;
                case Qt::ExtraButton3:
                    button = tr("Button %1").arg("3");
                    break;
                case Qt::ExtraButton4:
                    button = tr("Button %1").arg("4");
                    break;
                default:
                    button = tr("Button");
                    break;
            }

            if (event->type() == QEvent::MouseButtonPress) {
                ui->testClickButton->setText(tr("%1 Single Click").arg(button));
            } else {
                ui->testClickButton->setText(tr("%1 Double Click").arg(button));
            }
            return true;
        }
    }
    return false;
}

void MousePane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
