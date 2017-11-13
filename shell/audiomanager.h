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

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include <QTimer>
#include <tvariantanimation.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/volume.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>
#include <pulse/stream.h>

class AudioManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quietMode QuietMode READ QuietMode WRITE setQuietMode NOTIFY QuietModeChanged)
public:
    explicit AudioManager(QObject *parent = 0);

    enum quietMode {
        none,
        notifications,
        mute
    };

    int MasterVolume();
    quietMode QuietMode();
    QString getCurrentQuietModeDescription();

signals:
    void QuietModeChanged(quietMode mode);

public slots:
    void setMasterVolume(int volume);
    void changeVolume(int volume);
    void attenuateStreams();
    void silenceStreams();
    void restoreStreams(bool immediate = false);
    void setQuietMode(quietMode mode);

private:
    pa_context* pulseContext = NULL;
    pa_mainloop_api* pulseLoopApi = NULL;

    static void pulseStateChanged(pa_context *c, void *userdata);
    static void pulseGetSinks(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void pulseGetDefaultSink(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void pulseSubscribe(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata);
    static void pulseServerInfo(pa_context *c, const pa_server_info *i, void *userdata);
    static void pulseGetSources(pa_context *c, const pa_source_info *i, int eol, void *userdata);
    static void pulseGetInputSinks(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
    static void pulseGetClients(pa_context *c, const pa_client_info*i, int eol, void *userdata);

    QMap<int, pa_cvolume> originalStreamVolumes;
    int attenuateRequests = 0;

    bool pulseAvailable = false;
    bool attenuateMode = false;
    int defaultSinkIndex = -1;
    QList<uint> tsClientIndices;
    QList<uint> newTsClientIndices;
    pa_cvolume defaultSinkVolume;
    quietMode currentQuietMode = none;
    QSettings settings;
};

#endif // AUDIOMANAGER_H
