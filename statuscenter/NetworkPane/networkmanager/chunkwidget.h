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
#ifndef CHUNKWIDGET_H
#define CHUNKWIDGET_H

#include <QWidget>
#include <QLabel>

namespace Ui {
    class ChunkWidget;
}

class DevicePanel;
struct ChunkWidgetPrivate;
class ChunkWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ChunkWidget(QWidget *parent = nullptr);
        ~ChunkWidget();

        void setIcon(QIcon icon, bool isFlightMode = false);
        void setText(QString text);

        void setSupplementaryText(QString supplementaryText);

        void watch(DevicePanel* device);
        void endWatch();

        QLabel* snackWidget();

        void setVisible(bool visible);

    signals:
        void showNetworkPane();

    private:
        Ui::ChunkWidget *ui;
        ChunkWidgetPrivate* d;

        void mousePressEvent(QMouseEvent* event);
        void set();
};

#endif // CHUNKWIDGET_H
