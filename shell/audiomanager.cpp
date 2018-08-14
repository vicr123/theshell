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

    quietModeWatcher = new QTimer();
    quietModeWatcher->setInterval(1000);
    connect(quietModeWatcher, &QTimer::timeout, [=] {
        if (QDateTime::currentDateTime() > quietModeOff) {
            setQuietMode(none);
            quietModeWatcher->stop();
        }
    });
}

void AudioManager::changeVolume(int volume) {
    if (currentQuietMode != mute) {
        pa_volume_t avgVol = pa_cvolume_avg(&defaultSinkVolume);
        int onePercent = (PA_VOLUME_NORM - PA_VOLUME_MUTED) / 100;
        pa_volume_t newVol = avgVol + (onePercent * volume);
        if (newVol < PA_VOLUME_MUTED) newVol = PA_VOLUME_MUTED;
        if (newVol > PA_VOLUME_MAX) newVol = PA_VOLUME_MAX;

        if (!settings.value("sound/volumeOverdrive", true).toBool() && newVol > PA_VOLUME_NORM) {
            newVol = PA_VOLUME_NORM;
        }

        if (volume < 0) {
            if (newVol > avgVol) {
                newVol = PA_VOLUME_MUTED;
            }
        }
        if (avgVol < PA_VOLUME_NORM) {
            if (newVol > PA_VOLUME_NORM) {
                newVol = PA_VOLUME_NORM;
            }
        }


        pa_cvolume newCVol = defaultSinkVolume;
        for (int i = 0; i < newCVol.channels; i++) {
            newCVol.values[i] = newVol;
        }
        pa_context_set_sink_mute_by_index(pulseContext, defaultSinkIndex, false, NULL, NULL);
        pa_context_set_sink_volume_by_index(pulseContext, defaultSinkIndex, &newCVol, NULL, NULL);
    }
}

void AudioManager::setMasterVolume(int volume) {
    if (currentQuietMode != mute) {
        if (pulseAvailable) {
            pa_volume_t setVol = PA_VOLUME_MUTED + (((float) volume / 100) * (float) PA_VOLUME_NORM);
            pa_cvolume newVol = defaultSinkVolume;
            for (int i = 0; i < newVol.channels; i++) {
                newVol.values[i] = setVol;
            }
            pa_context_set_sink_volume_by_index(pulseContext, defaultSinkIndex, &newVol, NULL, NULL);
        } else {
            //Get Current Limits
            QProcess* mixer = new QProcess();
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
            pa_context_subscribe(c, (pa_subscription_mask_t) (PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE | PA_SUBSCRIPTION_MASK_SINK_INPUT | PA_SUBSCRIPTION_MASK_CLIENT), NULL, userdata);
            pa_context_get_server_info(c, &AudioManager::pulseServerInfo, currentManager);
            pa_context_get_sink_info_list(c, &AudioManager::pulseGetSinks, currentManager);
            pa_context_get_source_info_list(c, &AudioManager::pulseGetSources, currentManager);
            pa_context_get_sink_input_info_list(c, &AudioManager::pulseGetInputSinks, currentManager);
            pa_context_get_client_info_list(c, &AudioManager::pulseGetClients, currentManager);
    }
}

void AudioManager::pulseSubscribe(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK:
            pa_context_get_server_info(c, &AudioManager::pulseServerInfo, currentManager);
            pa_context_get_sink_info_list(c, &AudioManager::pulseGetSinks, currentManager);
            break;
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            pa_context_get_sink_input_info_list(c, &AudioManager::pulseGetInputSinks, currentManager);
            break;
        case PA_SUBSCRIPTION_EVENT_SOURCE:
            pa_context_get_server_info(c, &AudioManager::pulseServerInfo, currentManager);
            pa_context_get_source_info_list(c, &AudioManager::pulseGetSources, currentManager);
            break;
        case PA_SUBSCRIPTION_EVENT_CLIENT:
            pa_context_get_client_info_list(c, &AudioManager::pulseGetClients, currentManager);
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

void AudioManager::pulseGetSources(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    if (eol == 0) {

    }
}

void AudioManager::attenuateStreams() {
    if (pulseAvailable) {
        if (attenuateRequests == 0) {
            for (int source : originalStreamVolumes.keys()) {
                //Set volume to 50% of original
                pa_cvolume originalVolume = originalStreamVolumes.value(source);
                for (int i = 0; i < originalVolume.channels; i++) {
                    if (originalVolume.values[i] > PA_VOLUME_NORM) {
                        originalVolume.values[i] = PA_VOLUME_NORM / 2;
                    } else {
                        originalVolume.values[i] /= 2;
                    }
                }
                pa_context_set_sink_input_volume(pulseContext, source, &originalVolume, NULL, NULL);
            }
            attenuateMode = true;
        }
        attenuateRequests++;
    }
}

void AudioManager::silenceStreams() {
    if (pulseAvailable) {
        for (int source : originalStreamVolumes.keys()) {
            //Set volume to 10% of original
            pa_cvolume originalVolume = originalStreamVolumes.value(source);
            for (int i = 0; i < originalVolume.channels; i++) {
                originalVolume.values[i] /= 10;
            }
            pa_context_set_sink_input_volume(pulseContext, source, &originalVolume, NULL, NULL);
        }
        attenuateMode = true;
    }
}

void AudioManager::restoreStreams(bool immediate) {
    if (pulseAvailable) {
        attenuateRequests--;
        if (attenuateRequests == 0) {
            //Sounds much better if we delay by a second
            QTimer::singleShot(immediate ? 0 : 1000, [=]() {
                for (int source : originalStreamVolumes.keys()) {
                    //Restore volume of each stream
                    pa_cvolume originalVolume = originalStreamVolumes.value(source);
                    if (originalVolume.channels > 0) {
                        tVariantAnimation* anim = new tVariantAnimation;
                        anim->setStartValue(originalVolume.values[0] / 2);
                        pa_volume_t endValue = originalVolume.values[0];
                        if (endValue > PA_VOLUME_NORM) {
                            endValue = PA_VOLUME_NORM;
                        }
                        anim->setEndValue(endValue);
                        anim->setDuration(500);
                        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                            pa_cvolume originalVolume = originalStreamVolumes.value(source);
                            for (int i = 0; i < originalVolume.channels; i++) {
                                originalVolume.values[i] = value.toUInt();
                            }
                            pa_context_set_sink_input_volume(pulseContext, source, &originalVolume, NULL, NULL);
                        });
                        connect(anim, &tVariantAnimation::finished, [=]() {
                            attenuateMode = false;
                        });
                        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                        anim->start();
                    }

                }
            });
        } else if (attenuateRequests == -1) {
            attenuateRequests = 0;
        }
    }
}

void AudioManager::pulseGetInputSinks(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    if (eol == 0) {
        if (!currentManager->tsClientIndices.contains(i->client)) {
            if (currentManager->attenuateMode) {
                if (currentManager->originalStreamVolumes.contains(i->index)) {
                    pa_cvolume originalVolume = i->volume;
                    for (int i = 0; i < originalVolume.channels; i++) {
                        originalVolume.values[i] = originalVolume.values[i] * 2;
                    }
                    currentManager->originalStreamVolumes.insert(i->index, originalVolume);
                } else {
                    currentManager->originalStreamVolumes.insert(i->index, i->volume);
                }
            } else {
                currentManager->originalStreamVolumes.insert(i->index, i->volume);
            }
        }
    }
}

void AudioManager::pulseGetClients(pa_context *c, const pa_client_info *i, int eol, void *userdata) {
    AudioManager* currentManager = (AudioManager*) userdata;
    if (eol == 0) {
        if (QString::fromUtf8(i->name).toLower().contains("theshell") || QString::fromUtf8(i->name).toLower().contains("qtpulseaudio")) {
            currentManager->newTsClientIndices.append(i->index);
        }
    } else {
        currentManager->tsClientIndices = currentManager->newTsClientIndices;
        currentManager->newTsClientIndices.clear();
    }
}

void AudioManager::setQuietMode(quietMode mode) {
    if (mode != currentQuietMode) {
        quietMode oldQuietMode = this->currentQuietMode;
        if (mode == mute) {
            pa_context_set_sink_mute_by_index(pulseContext, defaultSinkIndex, 1, NULL, NULL);
        }

        this->currentQuietMode = mode;
        emit QuietModeChanged(mode);

        if (oldQuietMode == mute) {
            pa_context_set_sink_mute_by_index(pulseContext, defaultSinkIndex, 0, NULL, NULL);
        }
    }
}

AudioManager::quietMode AudioManager::QuietMode() {
    return this->currentQuietMode;
}

QString AudioManager::getCurrentQuietModeDescription() {
    switch (this->currentQuietMode) {
        case none:
            return tr("Allows all sounds from all apps, and notifications from all apps.");
        case critical:
            return tr("Ignores all notifications not marked as critical and those set to bypass Quiet Mode. Normal sounds will still be played.");
        case notifications:
            return tr("Ignores any notifications from all apps, except those set to bypass Quiet Mode. Normal sounds will still be played, and timers and reminders will still notify you, however, they won't play sounds.");
        case mute:
            return tr("Completely turns off all sounds and notifications from all apps, including those set to bypass Quiet Mode. Not even timers or reminders will notify you.");
    }
}

void AudioManager::setQuietModeResetTime(QDateTime time) {
    if (time.isValid()) {
        quietModeOff = time;
        quietModeWatcher->start();
    } else {
        quietModeWatcher->stop();
    }
}
