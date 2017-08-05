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

#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QIcon>

//#include <X11/Xlib.h>

typedef unsigned long Window;

class WmWindow
{

public:
    explicit WmWindow();

    QString title() const;
    void setTitle(QString);

    unsigned long PID() const;
    void setPID(unsigned long);
    Window WID() const;
    void setWID(Window);
    QIcon icon() const;
    void setIcon(QIcon);
    bool attention() const;
    void setAttention(bool attention);
    int desktop() const;
    void setDesktop(int desktop);
    bool isMinimized() const;
    void setMinimized(bool minimized);
    QRect geometry() const;
    void setGeometry(QRect geometry);
signals:

public slots:

private:
    QString winTitle = "";
    int id = 0;
    unsigned long pid = 0;
    Window wid = 0;
    QIcon ic;
    bool attn = false;
    int dk = 0;
    bool min = false;
    QRect geo;
};

#endif // WINDOW_H
