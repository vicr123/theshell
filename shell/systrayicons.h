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

#ifndef SYSTRAYICONS_H
#define SYSTRAYICONS_H


#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QX11Info>
#include <QDebug>
#include <QTimer>
#include <QWindow>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include "nativeeventfilter.h"

//Xlib needs to be included LAST.
#include <X11/Xlib.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

class SysTrayIcons : public QFrame
{
    Q_OBJECT
public:
    explicit SysTrayIcons(QWidget *parent = 0);

signals:

public slots:

private slots:
    void SysTrayEvent(long opcode, long data2, long data3, long data4);
    void SniItemRegistered(QString service);
    void SniItemUnregistered(QString service);

private:
    QStringList availableSniServices;
};

class SniIcon : public QLabel
{
    Q_OBJECT
public:
    explicit SniIcon(QString service, QWidget *parent = 0);

private slots:
    void SniItemUnregistered(QString service);
    void ReloadIcon();

private:
    QString service;
    QString title;
    QDBusInterface* interface;

    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
};

#endif // SYSTRAYICONS_H
