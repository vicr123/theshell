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

#ifndef TASKBARBUTTON_H
#define TASKBARBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <QLinearGradient>
#include <tvariantanimation.h>

struct TaskbarButtonPrivate;
class DesktopWmWindow;
typedef QPointer<DesktopWmWindow> DesktopWmWindowPtr;
class TaskbarButton : public QPushButton
{
    Q_OBJECT
    public:
        explicit TaskbarButton(DesktopWmWindowPtr window);

        void setText(QString text);
        void setIcon(QIcon icon);

        void adjustSize();
        void animateOut();

        QSize sizeHint() const;

    signals:

    public slots:

    private:
        TaskbarButtonPrivate* d;
        void paintEvent(QPaintEvent* event);

        void enterEvent(QEvent* event);
        void leaveEvent(QEvent* event);
};

#endif // TASKBARBUTTON_H
