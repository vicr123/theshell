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

#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QApplication>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(QWidget* parent);

    void setShowDisabled(bool showDisabled);
    bool showDisabled();
signals:
    void clicked();
    void dragging(int x, int y);
    void mouseReleased();

private:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    //void paintEvent(QPaintEvent *event);

    bool isShowDisabled = false;
    bool isClicked = false;
    bool didDrag = false;
};

#endif // CLICKABLELABEL_H
