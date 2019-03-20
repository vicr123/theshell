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
#ifndef AUDIOPANE_H
#define AUDIOPANE_H

#include <QWidget>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>

#include <statuscenterpaneobject.h>

namespace Ui {
    class AudioPane;
}

struct AudioPanePrivate;
class AudioPane : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit AudioPane(QWidget *parent = nullptr);
        ~AudioPane();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_backButton_clicked();

        void connectToPulse();

        void updateSinkMute(bool mute, int index);
        void updateSinkVolume(pa_cvolume, int index);
        void setDefaultSink(QString sink);

        void updateSinkInputMute(bool mute, int index);

    signals:
        void defaultSinkChanged(QString defaultSink);

    private:
        Ui::AudioPane *ui;

        AudioPanePrivate* d;

        static void pulseaudio_subscribe_callback(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* userdata);
        static void pulseaudio_state_callback(pa_context *c, void *userdata);

        static void updatePulseState(AudioPane* pane);

        static void addSink(AudioPane* pane, pa_sink_info info);
        static void updateSink(AudioPane* pane, int index);
        static void removeSink(AudioPane* pane, int index);

        static void addSinkInput(AudioPane* pane, pa_sink_input_info info);
        static void updateSinkInput(AudioPane* pane, int index);
        static void removeSinkInput(AudioPane* pane, int index);
};

#endif // AUDIOPANE_H
