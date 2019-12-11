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
#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include <QWidget>

namespace PulseAudioQt {
    class Source;
}

namespace Ui {
    class SourceWidget;
}

struct SourceWidgetPrivate;
class SourceWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SourceWidget(PulseAudioQt::Source* source, QWidget *parent = nullptr);
        ~SourceWidget();

        PulseAudioQt::Source* source();

    private slots:

        void on_muteButton_toggled(bool checked);

        void on_volumeSlider_valueChanged(int value);

        void on_defaultButton_toggled(bool checked);

    private:
        Ui::SourceWidget *ui;
        SourceWidgetPrivate* d;
};

#endif // SOURCEWIDGET_H
