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

#include <qsettingsformats.h>
#include <tsystemsound.h>

#include "sinkwidget.h"
#include "sinkinputwidget.h"

#include <Context>
#include <Server>
#include <Sink>
#include <SinkInput>

struct AudioPanePrivate {
    bool pulseAvailable = false;

    QString defaultSink;

    QList<SinkWidget*> sinkWidgets;
    QList<SinkInputWidget*> sinkInputWidgets;

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
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkAdded, this, [=](PulseAudioQt::Sink* sink) {
        //Add a new sink
        SinkWidget* w = new SinkWidget(sink);
        d->sinkWidgets.append(w);
        ui->sinksLayout->addWidget(w);
    });
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkRemoved, this, [=](PulseAudioQt::Sink* sink) {
        //Remove a sink
        for (SinkWidget* w : d->sinkWidgets) {
            if (w->sink() == sink) {
                d->sinkWidgets.removeAll(w);
                ui->sinksLayout->removeWidget(w);
                w->deleteLater();
                return;
            }
        }
    });
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkInputAdded, this, [=](PulseAudioQt::SinkInput* sinkInput) {
        //Add a sink input
        SinkInputWidget* w = new SinkInputWidget(sinkInput);
        d->sinkInputWidgets.append(w);
        ui->sinkInputsLayout->addWidget(w);
    });
    connect(PulseAudioQt::Context::instance(), &PulseAudioQt::Context::sinkInputRemoved, this, [=](PulseAudioQt::SinkInput* sinkInput) {
        //Remove a sink input
        for (SinkInputWidget* w : d->sinkInputWidgets) {
            if (w->sinkInput() == sinkInput) {
                d->sinkInputWidgets.removeAll(w);
                ui->sinkInputsLayout->removeWidget(w);
                w->deleteLater();
                return;
            }
        }
    });

    if (PulseAudioQt::Context::instance() == nullptr) {
        d->pulseAvailable = false;
    } else {
        d->pulseAvailable = true;
    }
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
