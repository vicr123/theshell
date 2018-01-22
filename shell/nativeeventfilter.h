/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <QX11Info>
#include <QProcess>
#include <QTime>
#include <math.h>
#include <QIcon>
#include "hotkeyhud.h"
#include "endsessionwait.h"
#include "dbusevents.h"
#include "rundialog.h"
#include "screenrecorder.h"
#include <QDBusUnixFileDescriptor>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QMessageBox>
#include <QSoundEffect>
#include "mainwindow.h"
#include "screenshotwindow.h"

#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

class MainWindow;

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject* parent = 0);
    ~NativeEventFilter();

signals:
    void SysTrayEvent(long opcode, long data2, long data3, long data4);

public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    QTime lastPress;
    HotkeyHud* Hotkeys;

    bool isEndSessionBoxShowing = false;
    bool ignoreSuper = false;

    QSettings settings;
    QSettings* themeSettings = new QSettings("theSuite", "ts-qtplatform");
};

#endif // NATIVEEVENTFILTER_H
