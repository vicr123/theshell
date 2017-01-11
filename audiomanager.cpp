#include "audiomanager.h"

AudioManager::AudioManager(QObject *parent) : QObject(parent)
{
    pulseLoopApi = pa_glib_mainloop_get_api(pa_glib_mainloop_new(NULL));

    pa_proplist* propList = pa_proplist_new();
    pa_proplist_sets(propList, PA_PROP_APPLICATION_NAME, "theShell");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_ID, "org.thesuite.theshell");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_ICON_NAME, "theshell");

    pulseContext = pa_context_new_with_proplist(pulseLoopApi, NULL, propList);
    pa_proplist_free(propList);
    pa_context_set_state_callback(pulseContext, &AudioManager::pulseStateChanged, this);

    int connected = pa_context_connect(pulseContext, NULL, PA_CONTEXT_NOFLAGS, NULL);

    if (connected < 0) {
        pulseAvailable = false;
    } else {
        pulseAvailable = true;
    }
}

void AudioManager::changeVolume(int volume) {
    pa_volume_t avgVol = pa_cvolume_avg(&defaultSinkVolume);
    int onePercent = (PA_VOLUME_NORM - PA_VOLUME_MUTED) / 100;
    avgVol += onePercent * volume;

    pa_cvolume newVol = defaultSinkVolume;
    for (int i = 0; i < newVol.channels; i++) {
        newVol.values[i] = avgVol;
    }
    pa_context_set_sink_volume_by_index(pulseContext, defaultSinkIndex, &newVol, NULL, NULL);
}

void AudioManager::setMasterVolume(int volume) {
    if (pulseAvailable) {
        pa_volume_t setVol = PA_VOLUME_MUTED + (((float) volume / 100) * (float) PA_VOLUME_NORM);
        pa_cvolume newVol = defaultSinkVolume;
        for (int i = 0; i < newVol.channels; i++) {
            newVol.values[i] = setVol;
        }
        pa_context_set_sink_volume_by_index(pulseContext, defaultSinkIndex, &newVol, NULL, NULL);
    } else {
        //Get Current Limits
        QProcess* mixer = new QProcess(this);
        mixer->start("amixer");
        mixer->waitForFinished();
        QString output(mixer->readAll());

        bool readLine = false;
        int limit;
        for (QString line : output.split("\n")) {
            if (line.startsWith(" ") && readLine) {
                if (line.startsWith("  Limits:")) {
                    limit = line.split(" ").last().toInt();
                }
            } else {
                if (line.contains("'Master'")) {
                    readLine = true;
                } else {
                    readLine = false;
                }
            }
        }

        mixer->start("amixer set Master " + QString::number(limit * (volume / (float) 100)) + " on");
        connect(mixer, SIGNAL(finished(int)), mixer, SLOT(deleteLater()));
    }
}

int AudioManager::MasterVolume() {
    if (pulseAvailable) {
        pa_volume_t avgVol = pa_cvolume_avg(&defaultSinkVolume);
        int currentVol = ((float) (avgVol - PA_VOLUME_MUTED) / (float) PA_VOLUME_NORM) * 100;
        return currentVol;
    } else {
        //Get Current Volume
        QProcess* mixer = new QProcess(this);
        mixer->start("amixer");
        mixer->waitForFinished();
        QString output(mixer->readAll());
        delete mixer;

        bool readLine = false;
        for (QString line : output.split("\n")) {
            if (line.startsWith(" ") && readLine) {
                if (line.startsWith("  Front Left:")) {
                    if (line.contains("[off]")) {
                        return 0;
                    } else {
                        QString percent = line.mid(line.indexOf("\[") + 1, 3).remove("\%").remove("]");
                        return percent.toInt();
                    }
                }
            } else {
                if (line.contains("'Master'")) {
                    readLine = true;
                } else {
                    readLine = false;
                }
            }
        }
    }
    return 0;
}

void AudioManager::pulseStateChanged(pa_context *c, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY:
            pa_context_set_subscribe_callback(c, &AudioManager::pulseSubscribe, currentManager);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, userdata);
            pa_context_get_server_info(c, &AudioManager::pulseServerInfo, currentManager);
            pa_context_get_sink_info_list(c, &AudioManager::pulseGetSinks, currentManager);
    }
}

void AudioManager::pulseSubscribe(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            pa_context_get_server_info(c, &AudioManager::pulseServerInfo, currentManager);
            pa_context_get_sink_info_list(c, &AudioManager::pulseGetSinks, currentManager);
    }
}

void AudioManager::pulseGetSinks(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {

}

void AudioManager::pulseServerInfo(pa_context *c, const pa_server_info *i, void *userdata) {
    pa_context_get_sink_info_by_name(c, i->default_sink_name, &AudioManager::pulseGetDefaultSink, userdata);
}

void AudioManager::pulseGetDefaultSink(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    if (eol == 0) {
        AudioManager* currentManager = (AudioManager*) userdata;
        currentManager->defaultSinkIndex = i->index;
        currentManager->defaultSinkVolume = i->volume;
    }
}
