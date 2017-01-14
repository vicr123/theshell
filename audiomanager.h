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
public:
    explicit AudioManager(QObject *parent = 0);

    int MasterVolume();

signals:

public slots:
    void setMasterVolume(int volume);
    void changeVolume(int volume);
    void quietStreams();
    void restoreStreams();

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

    bool pulseAvailable = false;
    bool quietMode = false;
    int defaultSinkIndex = -1;
    QList<uint> tsClientIndices;
    QList<uint> newTsClientIndices;
    pa_cvolume defaultSinkVolume;
};

#endif // AUDIOMANAGER_H
