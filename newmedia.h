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

#ifndef NEWMEDIA_H
#define NEWMEDIA_H

#include <QDialog>
#include <QProcess>
#include <QX11Info>
#include <QDesktopWidget>
#include <tpropertyanimation.h>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QToolButton>

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
class NewMedia;
}

class NewMedia : public QDialog
{
    Q_OBJECT

public:
    explicit NewMedia(QString description, QWidget *parent = 0);
    ~NewMedia();

    void show();
    void close();
    void reject();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);
private slots:
    void on_closeButton_clicked();

    void on_doNothingOptionToolButton_clicked();

private:
    Ui::NewMedia *ui;

    void paintEvent(QPaintEvent* event);
};

#endif // NEWMEDIA_H
