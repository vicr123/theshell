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
#ifndef SINKWIDGET_H
#define SINKWIDGET_H

#include <QWidget>
#include <pulse/introspect.h>

namespace PulseAudioQt {
    class Sink;
}

namespace Ui {
    class SinkWidget;
}

struct SinkWidgetPrivate;
class SinkWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SinkWidget(PulseAudioQt::Sink* sink, QWidget *parent = nullptr);
        ~SinkWidget();

        PulseAudioQt::Sink* sink();

        QString currentPort();

        void updateInfo(pa_sink_info info, QString defaultSinkName);

    public slots:
        void defaultSinkChanged(QString defaultSinkName);

    private slots:
        void on_muteButton_toggled(bool checked);

        void on_expandVolumesButton_clicked();

        void on_volumeSlider_sliderPressed();

        void on_volumeSlider_sliderReleased();

        void on_volumeSlider_valueChanged(int value);

        void on_defaultButton_toggled(bool checked);

        void updateChannels();

        void updateChannelVolumes();

        void updatePorts();

    private:
        Ui::SinkWidget *ui;

        SinkWidgetPrivate* d;
};

#endif // SINKWIDGET_H
