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

#ifndef MOUSESCROLLWIDGET_H
#define MOUSESCROLLWIDGET_H

#include <QScrollArea>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QScrollBar>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QSettings>

class MouseScrollWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit MouseScrollWidget(QWidget *parent = 0);

    //QSize sizeHint() const;
    void setWidget(QWidget *widget);
signals:

public slots:

private:
    QTimer shiftLeft, shiftRight;
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

    void setEventFilter(QWidget* widget);
    void setEventFilter(QLayout* layout);

    QSize sizeHint() const;

    QWidget* vp;
    QSettings settings;

    int scrollSpeed = 0;
};

#endif // MOUSESCROLLWIDGET_H
