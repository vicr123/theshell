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
#ifndef SINKINPUTWIDGET_H
#define SINKINPUTWIDGET_H

#include <QWidget>
#include <pulse/introspect.h>

namespace Ui {
    class SinkInputWidget;
}

class SinkInputWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SinkInputWidget(pa_context* ctx, QWidget *parent = nullptr);
        ~SinkInputWidget();

        int paIndex();

        void updateInfo(pa_sink_input_info info);

    signals:
        void updateMute(bool mute, int index);

    private slots:

        void on_muteButton_toggled(bool checked);

    private:
        Ui::SinkInputWidget *ui;

        int lastIndex = 0;
        pa_context* ctx;
};

#endif // SINKINPUTWIDGET_H
