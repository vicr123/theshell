/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef DISPLAYARRANGEMENTWIDGET_H
#define DISPLAYARRANGEMENTWIDGET_H

#include <QWidget>

namespace Ui {
    class DisplayArrangementWidget;
}

struct DisplayArrangementWidgetPrivate;
class DisplayArrangementWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit DisplayArrangementWidget(unsigned long output, QWidget *parent = nullptr);
        ~DisplayArrangementWidget();

        QRect requestedGeometry();
        void offset(QPoint distance);
        bool powered();

    public slots:
        void doPosition(QPoint origin);
        void set();
        void setDefaultOutput(bool isDefault);

    signals:
        void configureMe(QWidget* configurator);
        void setDefault();
        void setOtherDefault();

    private:
        Ui::DisplayArrangementWidget *ui;

        DisplayArrangementWidgetPrivate* d;

        void paintEvent(QPaintEvent* paintEvent);
        void mousePressEvent(QMouseEvent* event);
        void mouseMoveEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);
};

#endif // DISPLAYARRANGEMENTWIDGET_H
