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

#include <math.h>
#include <QSettings>
#include <QX11Info>
#include <QDebug>

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
                if (value.first().type() == QVariant::Bool || value.first().type() == QVariant::Int) {
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
                } else if (value.first().type() == QVariant::Int && formatReturn != 8) {
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
                } else if (value.first().type() == QVariant::Int) {
                    unsigned char v[64];
                    for (int i = 0; i < value.count(); i++) {
                        v[i] = value.at(i).toInt();
                    }
                    XIChangeProperty(QX11Info::display(), d->id, valAtom, type, 8, XIPropModeReplace, reinterpret_cast<unsigned char*>(v), value.count());
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

    if (XInternAtom(QX11Info::display(), LIBINPUT_PROP_ACCEL, True)) {
        //We have libinput
        qDebug() << "Using libinput";
        d->useLibInput = true;
    }

    unsigned char pointerMapping[256];
    d->numButtons = XGetPointerMapping(QX11Info::display(), pointerMapping, 256);
    if (d->numButtons >= 3) {
        d->middleButton = pointerMapping[1];
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
            d->writeXiSetting(SYNAPTICS_PROP_TAP_ACTION, {0, 0, 0, 0, 1, 0, 0}, MousePanePrivate::Touchpads);
        } else {
            d->writeXiSetting(LIBINPUT_PROP_TAP, {false}, MousePanePrivate::Touchpads);
            d->writeXiSetting(SYNAPTICS_PROP_TAP_ACTION, {0, 0, 0, 0, 0, 0, 0}, MousePanePrivate::Touchpads);
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
