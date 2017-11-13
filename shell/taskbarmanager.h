/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef TASKBARMANAGER_H
#define TASKBARMANAGER_H

#include <QObject>
#include <QMap>
#include <QX11Info>
#include <QApplication>
#include <QSettings>
#include "window.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#undef Bool

class TaskbarManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskbarManager(QObject *parent = nullptr);

    QList<WmWindow> Windows();
signals:
    void windowsChanged();
    void updateWindow(WmWindow changedWindow);
    void deleteWindow(WmWindow closedWindow);

public slots:
    void ReloadWindows();

private slots:
    bool updateInternalWindow(Window window);

private:
    QMap<Window, WmWindow> knownWindows;

    QSettings settings;
};

#endif // TASKBARMANAGER_H
