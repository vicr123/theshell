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

#ifndef HOTKEYHUD_H
#define HOTKEYHUD_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#undef Status
#undef Bool
#undef None
#undef FocusIn
#undef FontChange
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusOut
#undef CursorShape
#undef Unsorted

namespace Ui {
class HotkeyHud;
}

class HotkeyHud : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit HotkeyHud(QWidget *parent = 0);
    ~HotkeyHud();

    void show(QIcon icon, QString control, int value);
    void show(QIcon icon, QString control, QString explanation, int timeout = 1500);
    void close();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

private slots:
    void Timeout();

private:
    Ui::HotkeyHud *ui;

    void paintEvent(QPaintEvent* event);
    void show(int timeout = 1500);
    bool isShowing = false;

    int value;

    QTimer* timeout = NULL;
};

#endif // HOTKEYHUD_H
