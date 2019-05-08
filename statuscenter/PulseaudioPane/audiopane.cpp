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
#include "audiopane.h"
#include "ui_audiopane.h"

#include <the-libs_global.h>
#include <QDir>

#include <pulse/context.h>
#include <pulse/subscribe.h>
#include <pulse/glib-mainloop.h>
#include <pulse/introspect.h>

#include <qsettingsformats.h>
#include <tsystemsound.h>

#include "sinkwidget.h"
#include "sinkinputwidget.h"

struct AudioPanePrivate {
    pa_context* ctx = nullptr;
    pa_mainloop_api* mainloopapi = nullptr;

    bool pulseAvailable = false;

    QString defaultSink;

    QMap<QString, SinkWidget*> sinkWidgets;
    QMap<QString, SinkInputWidget*> sinkInputWidgets;

    QSettings settings;
    int currentSoundSettingRow = 0;
};

AudioPane::AudioPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioPane)
{
    ui->setupUi(this);
    d = new AudioPanePrivate();

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-sound");
    this->settingAttributes.menuWidget = ui->LeftPaneWidget;
    this->settingAttributes.providesLeftPane = true;

    ui->LeftPaneWidget->setFixedWidth(250 * theLibsGlobal::getDPIScaling());

    ui->audioStack->setCurrentAnimation(tStackedWidget::Lift);

    //Populate the sound theme box
    QSignalBlocker blocker(ui->soundThemeComboBox);
    QStringList foundThemes;
    QStringList themeSearchPaths = {
        QDir::homePath() + "/.local/share/sounds",
        "/usr/share/sounds"
    };

    QStringList folderSearches;
    for (QString searchPath : themeSearchPaths) {
        QDir dir(searchPath);
        for (QString folderName : dir.entryList(QDir::Dirs)) {
            QDir themeDir(dir.absoluteFilePath(folderName));
            if (themeDir.exists("index.theme")) {
                //Read in the index.theme file
                QSettings themeFile(themeDir.absoluteFilePath("index.theme"), QSettingsFormats::desktopFormat());
                QString theme = themeFile.value("Sound Theme/Name").toString();
                if (!foundThemes.contains(theme)) foundThemes.append(theme);
            }
        }
    }
    ui->soundThemeComboBox->addItems(foundThemes);

    QSettings platformSettings("theSuite", "ts-qtplatform");
    QString soundTheme = platformSettings.value("sound/theme", "Contemporary").toString();
    ui->soundThemeComboBox->setCurrentIndex(foundThemes.indexOf(soundTheme));

    ui->volumeOverdrive->setChecked(d->settings.value("sound/volumeOverdrive", true).toBool());

    addSoundSetting(tr("Login"), "desktop-login", "login");
    addSoundSetting(tr("Logout"), "desktop-logout", "logout");
    addSoundSetting(tr("Information"), "dialog-information", "dialoginfo");
    addSoundSetting(tr("Question"), "dialog-question", "dialogquestion");
    addSoundSetting(tr("Warning"), "dialog-warning", "dialogwarn");
    addSoundSetting(tr("Error"), "dialog-error", "dialogerr");
    addSoundSetting(tr("Screenshot"), "screen-capture", "screenshot");
    addSoundSetting(tr("Volume Change"), "audio-volume-change", "volfeedback");

    connectToPulse();
}

AudioPane::~AudioPane()
{
    delete d;
    delete ui;
}

QWidget* AudioPane::mainWidget() {
    return this;
}

QString AudioPane::name() {
    return tr("Audio");
}

StatusCenterPaneObject::StatusPaneTypes AudioPane::type() {
    return Setting;
}

int AudioPane::position() {
    return 0;
}

void AudioPane::message(QString name, QVariantList args) {

}

void AudioPane::on_backButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}

void AudioPane::addSoundSetting(QString name, QString soundName, QString soundPermission) {
    QLabel* label = new QLabel();
    label->setText(name);

    tSwitch* s = new tSwitch();
    s->setChecked(tSystemSound::isSoundEnabled(soundPermission));
    connect(s, &tSwitch::toggled, [=](bool checked) {
        tSystemSound::setSoundEnabled(soundPermission, checked);
    });

    QPushButton* playButton = new QPushButton();
    playButton->setIcon(QIcon::fromTheme("audio-volume-high"));
    playButton->setFlat(true);
    connect(playButton, &QPushButton::clicked, [=] {
        tSystemSound::play(soundName);
    });

    QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    ui->soundsGridLayout->addWidget(label, d->currentSoundSettingRow, 0);
    ui->soundsGridLayout->addWidget(s, d->currentSoundSettingRow, 1);
    ui->soundsGridLayout->addWidget(playButton, d->currentSoundSettingRow, 2);
    ui->soundsGridLayout->addItem(spacer, d->currentSoundSettingRow, 3);

    d->currentSoundSettingRow++;
}

void AudioPane::connectToPulse() {
    //Establish a connection to PulseAudio
    d->mainloopapi = pa_glib_mainloop_get_api(pa_glib_mainloop_new(nullptr));

    pa_proplist* propList = pa_proplist_new();
    pa_proplist_sets(propList, PA_PROP_APPLICATION_NAME, "theShell");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_ID, "org.thesuite.theshell");
    pa_proplist_sets(propList, PA_PROP_APPLICATION_ICON_NAME, "theshell");

    d->ctx = pa_context_new_with_proplist(d->mainloopapi, nullptr, propList);
    pa_proplist_free(propList);
    pa_context_set_state_callback(d->ctx, &pulseaudio_state_callback, this);

    int connected = pa_context_connect(d->ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

    if (connected < 0) {
        d->pulseAvailable = false;
    } else {
        d->pulseAvailable = true;
    }
}

void AudioPane::pulseaudio_subscribe_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata) {
    AudioPane* pane = static_cast<AudioPane*>(userdata);
    if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE) {
        if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
            updateSink(pane, idx);
        } else if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
            updateSinkInput(pane, idx);
        } else if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SERVER) {
            updatePulseState(pane);
        }
    } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_NEW) {
        if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
            pa_context_get_sink_info_by_index(pane->d->ctx, idx, [](pa_context* c, const pa_sink_info* infoArray, int eol, void* userdata) {
                if (infoArray) {
                    addSink(static_cast<AudioPane*>(userdata), *infoArray);
                }
            }, pane);
        } else if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
            pa_context_get_sink_input_info(pane->d->ctx, idx, [](pa_context* c, const pa_sink_input_info* infoArray, int eol, void* userdata) {
                if (infoArray) {
                    addSinkInput(static_cast<AudioPane*>(userdata), *infoArray);
                }
            }, pane);
        }
    } else if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
        if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK) {
            removeSink(pane, idx);
        } else if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
            removeSinkInput(pane, idx);
        }
    }
}

void AudioPane::pulseaudio_state_callback(pa_context *c, void *userdata) {
    AudioPane* pane = static_cast<AudioPane*>(userdata);
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY:
            //Subscribe to events
            pa_context_set_subscribe_callback(pane->d->ctx, &pulseaudio_subscribe_callback, pane);
            pa_context_subscribe(pane->d->ctx, PA_SUBSCRIPTION_MASK_ALL, nullptr, nullptr);

            updatePulseState(pane);

            //Set up all the displays
            pa_context_get_sink_input_info_list(pane->d->ctx, [](pa_context* c, const pa_sink_input_info* infoArray, int eol, void* userdata) {
                if (infoArray) {
                    addSinkInput(static_cast<AudioPane*>(userdata), *infoArray);
                }
            }, pane);
            pa_context_get_sink_info_list(pane->d->ctx, [](pa_context* c, const pa_sink_info* infoArray, int eol, void* userdata) {
                if (infoArray) {
                    addSink(static_cast<AudioPane*>(userdata), *infoArray);
                }
            }, pane);
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            pane->d->pulseAvailable = false;
    }
}

void AudioPane::addSink(AudioPane* pane, pa_sink_info info) {
    SinkWidget* w = new SinkWidget();
    pane->d->sinkWidgets.insert(QString::fromLocal8Bit(info.name), w);
    pane->ui->sinksLayout->addWidget(w);

    connect(w, &SinkWidget::updateMute, pane, &AudioPane::updateSinkMute);
    connect(w, &SinkWidget::updateVolume, pane, &AudioPane::updateSinkVolume);
    connect(w, &SinkWidget::setDefault, pane, &AudioPane::setDefaultSink);
    connect(pane, &AudioPane::defaultSinkChanged, w, &SinkWidget::defaultSinkChanged);

    w->updateInfo(info, pane->d->defaultSink);
}

void AudioPane::updateSink(AudioPane *pane, int index) {
    pa_context_get_sink_info_by_index(pane->d->ctx, index, [](pa_context* c, const pa_sink_info* infoArray, int eol, void* userdata) {
        if (infoArray) {
            AudioPane* pane = static_cast<AudioPane*>(userdata);
            pane->d->sinkWidgets.value(QString::fromLocal8Bit(infoArray->name))->updateInfo(*infoArray, pane->d->defaultSink);
        }
    }, pane);
}

void AudioPane::removeSink(AudioPane *pane, int index) {
   for (QString name : pane->d->sinkWidgets.keys()) {
       SinkWidget* w = pane->d->sinkWidgets.value(name);
       if (w->paIndex() == index) {
           pane->d->sinkWidgets.remove(name);
           pane->ui->sinksLayout->removeWidget(w);
           w->deleteLater();
           return;
       }
   }
}

void AudioPane::addSinkInput(AudioPane *pane, pa_sink_input_info info) {
    SinkInputWidget* w = new SinkInputWidget(pane->d->ctx);
    pane->d->sinkInputWidgets.insert(QString::fromLocal8Bit(info.name), w);
    pane->ui->sinkInputsLayout->addWidget(w);

    connect(w, &SinkInputWidget::updateMute, pane, &AudioPane::updateSinkInputMute);

    w->updateInfo(info);
}

void AudioPane::updateSinkInput(AudioPane *pane, int index) {
    pa_context_get_sink_input_info(pane->d->ctx, index, [](pa_context* c, const pa_sink_input_info* infoArray, int eol, void* userdata) {
        if (infoArray) {
            AudioPane* pane = static_cast<AudioPane*>(userdata);

            QString sinkName = QString::fromLocal8Bit(infoArray->name);
            if (pane->d->sinkInputWidgets.contains(sinkName)) {
                pane->d->sinkInputWidgets.value(sinkName)->updateInfo(*infoArray);
            }
        }
    }, pane);
}

void AudioPane::removeSinkInput(AudioPane *pane, int index) {
    for (QString name : pane->d->sinkInputWidgets.keys()) {
        SinkInputWidget* w = pane->d->sinkInputWidgets.value(name);
        if (w->paIndex() == index) {
            pane->d->sinkInputWidgets.remove(name);
            pane->ui->sinkInputsLayout->removeWidget(w);
            w->deleteLater();
            return;
        }
    }
}

void AudioPane::updateSinkMute(bool mute, int index) {
    pa_context_set_sink_mute_by_index(d->ctx, index, mute, nullptr, nullptr);
}

void AudioPane::updateSinkInputMute(bool mute, int index) {
    pa_context_set_sink_input_mute(d->ctx, index, mute, nullptr, nullptr);
}

void AudioPane::updateSinkVolume(pa_cvolume volume, int index) {
    pa_context_set_sink_volume_by_index(d->ctx, index, &volume, nullptr, nullptr);
}

void AudioPane::updatePulseState(AudioPane *pane) {
    pa_context_get_server_info(pane->d->ctx, [](pa_context* c, const pa_server_info* infoArray, void* userdata) {
        AudioPane* pane = static_cast<AudioPane*>(userdata);
        pane->d->defaultSink = QString::fromLocal8Bit(infoArray->default_sink_name);

        emit pane->defaultSinkChanged(pane->d->defaultSink);
    }, pane);
}

void AudioPane::setDefaultSink(QString sink) {
    pa_context_set_default_sink(d->ctx, sink.toLocal8Bit().data(), nullptr, nullptr);
}

void AudioPane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void AudioPane::on_volumeOverdrive_toggled(bool checked)
{
    d->settings.setValue("sound/volumeOverdrive", checked);
}

void AudioPane::on_soundThemeComboBox_currentIndexChanged(int index)
{
    QSettings platformSettings("theSuite", "ts-qtplatform");
    platformSettings.setValue("sound/theme", ui->soundThemeComboBox->itemText(index));
}
