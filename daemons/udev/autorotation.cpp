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

#include "autorotation.h"

#include <QDebug>
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>
#include "daemon.h"

AutoRotation::AutoRotation(Daemon *parent) : QObject(parent)
{
    daemon = parent;

    //Create udev context
    context = udev_new();
    if (!context) return; //Error occurred, bail out

    //Enumerate over all devices
    udev_enumerate* enumerate = udev_enumerate_new(context);
    if (!enumerate) return; //Error occurred, bail out

    udev_enumerate_add_match_subsystem(enumerate, "iio");
    udev_enumerate_scan_devices(enumerate);

    udev_list_entry* devices;
    devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) return;

    udev_list_entry* device;
    udev_list_entry_foreach(device, devices) {
        const char* path = udev_list_entry_get_name(device);
        udev_device* dev = udev_device_new_from_syspath(context, path);

        if (QString::fromLocal8Bit(udev_device_get_sysattr_value(dev, "name")) == "accel_3d") { //yay we found an accelerometer
            accelerometerPath = QString::fromLocal8Bit(path);
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);

    //Get the primary display
    //Very hacky; might be a better way to do this
    QProcess* randr = new QProcess();
    randr->start("xrandr --listmonitors");
    connect(randr, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
        QString output = randr->readAll();
        QStringList lines = output.split("\n");
        lines.takeFirst();
        primaryDisplay = lines.first().split(" ").last();

        randr->deleteLater();
    });

    //Listen for screen change events
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(remapTouchScreens()));

    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkRotation()));

    if (accelerometerPath != "") {
        timer->start();

        QTimer::singleShot(0, [=] {
            daemon->sendMessage("register-switch", QVariantList() << tr("Rotation Lock") << false);
        });
    }
}

int twosComplement(int value) {
    if ((value & (1 << (16 - 1))) != 0) {
        value = value - (1 << 16);
    }
    return value;
}

void AutoRotation::checkRotation() {
    udev_device* accelerometer = udev_device_new_from_syspath(context, accelerometerPath.toLocal8Bit().constData());
    const char* xPointer = udev_device_get_sysattr_value(accelerometer, "in_accel_x_raw");
    const char* yPointer = udev_device_get_sysattr_value(accelerometer, "in_accel_y_raw");
    const char* zPointer = udev_device_get_sysattr_value(accelerometer, "in_accel_z_raw");

    if (xPointer == nullptr || yPointer == nullptr || zPointer == nullptr) return; //Bail out here

    int x = twosComplement(QString::fromLocal8Bit(xPointer).toInt());
    int y = twosComplement(QString::fromLocal8Bit(yPointer).toInt());
    int z = twosComplement(QString::fromLocal8Bit(zPointer).toInt());

    int absX = abs(x);
    int absY = abs(y);
    int absZ = abs(z);

    QString orientation;
    if (absZ > 4 * absX && absZ > 4 * absY) {
        orientation = "flat";
    } else if (3 * absY > 2 * absX) {
        if (y > 0) {
            orientation = "inverted";
        } else {
            orientation = "normal";
        }
    } else if (x > 0) {
        orientation = "left";
    } else {
        orientation = "right";
    }

    udev_device_unref(accelerometer);
    if (orientation != "flat" && orientation != oldOrientation) {
        oldOrientation = orientation;

        //Rotate the primary display
        QProcess::startDetached(QString("xrandr --output %1 --rotate %2").arg(primaryDisplay, orientation));
    }
}

void AutoRotation::remapTouchScreens() {
    //Create a list of inputs to remap
    //These are the devices I know, if you know any more, please send in a PR :)
    QStringList inputsToRemap = {
        "NTRG0001:01 1B96:1B05", //Surface Touch Screen
        "NTRG0001:01 1B96:1B05 Pen" //Surface Pen
    };

    QProcess* xinput = new QProcess();
    xinput->start("xinput list");
    connect(xinput, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
        QString output = xinput->readAll();
        QStringList lines = output.split("\n");
        for (QString line : lines) {
            for (QString input : inputsToRemap) {
                if (line.contains(input)) {
                    QStringList parts = line.split("\t", QString::SkipEmptyParts);
                    for (QString part : parts) {
                        if (part.startsWith("id=")) {
                            QString id = part.mid(3);
                            QProcess::startDetached(QString("xinput map-to-output %1 %2").arg(id, primaryDisplay));
                        }
                    }
                    break;
                }
            }
        }
        xinput->deleteLater();
    });
}

void AutoRotation::message(QString name, QVariantList args) {
    if (name == "switch-registered") {
        if (args.last().toString() == tr("Rotation Lock")) {
            switchId = args.first().toUInt();
        }
    } else if (name == "switch-toggled") {
        if (args.first().toUInt() == switchId) {
            if (!args.last().toBool()) {
                timer->start();
            } else {
                timer->stop();
            }
        }
    }
}
