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

#include "upowerdbus.h"
#include "power_adaptor.h"

#include <tpromise.h>
#include <tsystemsound.h>
#include "hotkeyhud.h"


extern void EndSession(EndSessionWait::shutdownType type);

UPowerDBus::UPowerDBus(QObject *parent) : QObject(parent)
{
    //Disable DPMS timeouts
    DPMSSetTimeouts(QX11Info::display(), 0, 0, 0);
}

//void UPowerDBus::checkUpower() {
//    QDBusInterface *i = new QDBusInterface("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", QDBusConnection::systemBus(), this);
//    if (i->property("LidIsClosed").toBool()) { //Is the lid closed?
//        if (!this->isLidClosed) { //Has the value changed?
//            this->isLidClosed = true;
//            if (QApplication::screens().count() == 1) { //How many monitors do we have?
//                //If we only have one montior, suspend the PC.
//                EndSession(EndSessionWait::suspend);
//            }
//        }
//    } else {
//        if (this->isLidClosed) {
//            this->isLidClosed = false;
//        }
//    }
//    delete i;
//}

void UPowerDBus::queryIdleState() {
    XScreenSaverInfo *info = new XScreenSaverInfo;
    if (!XScreenSaverQueryInfo(QX11Info::display(), DefaultRootWindow(QX11Info::display()), info)) {
        return;
    }

    if (info->idle < 3000) {
        idleScreen = false;
        idleSuspend = false;
    }

    CARD16 mode;
    BOOL isDpmsOn;
    DPMSInfo(QX11Info::display(), &mode, &isDpmsOn);

/*    if (charging()) {
        int idleTime = settings.value("power/powerScreenOff", 15).toInt();
        if (info->idle > idleTime * 60000 && !idleScreen && idleTime != 121) {
            //Don't turn the screen back on if it's already off
            if (isDpmsOn && mode != DPMSModeOff) {
                idleScreen = true;
                EndSession(EndSessionWait::screenOff);
            }
        }
        int suspendTime = settings.value("power/powerSuspend", 15).toInt() * 1000;
        if (info->idle > suspendTime * 60000 && !idleSuspend && suspendTime != 121) {
            idleSuspend = true;
            EndSession(EndSessionWait::suspend);
        }
    } else */{
        int idleTime = settings.value("power/batteryScreenOff", 15).toInt();
        if (info->idle > idleTime * 60000 && !idleScreen && idleTime != 121) {
            //Don't turn the screen back on if it's already off
            if (isDpmsOn && mode != DPMSModeOff) {
                idleScreen = true;
                EndSession(EndSessionWait::screenOff);
            }
        }
        int suspendTime = settings.value("power/batterySuspend", 15).toInt() * 1000;
        if (info->idle > suspendTime * 60000 && !idleSuspend && suspendTime != 121) {
            idleSuspend = true;
            EndSession(EndSessionWait::suspend);
        }
    }

    delete info;
}
