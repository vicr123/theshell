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
#include "sinkwidget.h"
#include "ui_sinkwidget.h"

#include <tvariantanimation.h>
#include "maps.h"

#include <QThread>
#include <tpromise.h>
#include <QMutex>
#include <Context>
#include <Sink>

struct SinkWidgetPrivate {
    PulseAudioQt::Sink* sink;
    int lastIndex = 0;
    int masterTracking = 0;

    bool movingMaster = false;
    QList<double> masterScales;

    QString sinkName;
    QMutex volumeChangeLocker;
};

SinkWidget::SinkWidget(PulseAudioQt::Sink* sink, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SinkWidget)
{
    ui->setupUi(this);

    d = new SinkWidgetPrivate;
    d->sink = sink;

    ui->volumeSlider->setMaximum(PulseAudioQt::normalVolume() * 1.5);
    ui->volumeSlider->setPageStep(PulseAudioQt::normalVolume() / 20);

    ui->allVolumesWidget->setFixedHeight(0);

    connect(d->sink, &PulseAudioQt::Sink::descriptionChanged, this, [=] {
        ui->deviceName->setText(sink->description());
    });
    connect(d->sink, &PulseAudioQt::Sink::mutedChanged, this, [=] {
        ui->muteButton->setChecked(sink->isMuted());
    });
    connect(d->sink, &PulseAudioQt::Sink::channelsChanged, this, &SinkWidget::updateChannels);
    connect(d->sink, &PulseAudioQt::Sink::channelVolumesChanged, this, &SinkWidget::updateChannelVolumes);

    ui->deviceName->setText(sink->description());
    ui->muteButton->setChecked(sink->isMuted());

    updateChannels();
    updateChannelVolumes();
}

SinkWidget::~SinkWidget()
{
    delete d;
    delete ui;
}

void SinkWidget::updateInfo(pa_sink_info info, QString defaultSinkName) {
    ui->deviceName->setText(QString::fromLocal8Bit(info.description).toUpper());
    ui->muteButton->setChecked(info.mute);

    d->lastIndex = info.index;
    d->sinkName = QString::fromLocal8Bit(info.name);

    defaultSinkChanged(defaultSinkName);
}

void SinkWidget::updateChannels() {
    QLayoutItem* item;
    while ((item = ui->volumesLayout->takeAt(0))) {
        ui->volumesLayout->removeItem(item);
        item->widget()->deleteLater();
    }

    for (int i = 0; i < d->sink->channels().count(); i++) {
        int speakerIndex = i;
        QString text = tr("Speaker");
//        if (channelPositionToString.contains(d->sink->channels().at(i)) {
//            text = QApplication::translate("maps", channelPositionToString.value(info.channel_map.map[speakerIndex]));
//        }
        text = d->sink->channels().at(i);

        QLabel* label = new QLabel();
        label->setText(text);
        label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        ui->volumesLayout->addWidget(label, speakerIndex, 0);

        int channelIndex = i;
        QSlider* slider = new QSlider();
        slider->setMaximum(PulseAudioQt::normalVolume() * 1.5);
        slider->setPageStep(PulseAudioQt::normalVolume() / 20);
        slider->setOrientation(Qt::Horizontal);
        slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(slider, &QSlider::valueChanged, slider, [=](int value) {
            d->sink->setChannelVolume(channelIndex, value);
        });
        ui->volumesLayout->addWidget(slider, speakerIndex, 1);
    }

    if (ui->allVolumesWidget->height() != 0) ui->allVolumesWidget->setFixedHeight(ui->allVolumesWidget->sizeHint().height());
}

void SinkWidget::updateChannelVolumes()
{
    int maxVolumeIndex = 0;
    for (int i = 0; i < d->sink->channelVolumes().count(); i++) {
        qreal volume = d->sink->channelVolumes().at(i);
        QSlider* slider = static_cast<QSlider*>(ui->volumesLayout->itemAtPosition(i, 1)->widget());
        QSignalBlocker blocker(slider);
        slider->setValue(volume);

        if (d->sink->channelVolumes().at(maxVolumeIndex) < volume) {
            maxVolumeIndex = i;
        }
    }

    if (!d->movingMaster) {
        QSignalBlocker blocker(ui->volumeSlider);
        ui->volumeSlider->setValue(d->sink->channelVolumes().at(maxVolumeIndex));
        d->masterTracking = maxVolumeIndex;
    }
}

void SinkWidget::defaultSinkChanged(QString defaultSinkName) {
    ui->defaultButton->setChecked(d->sinkName == defaultSinkName);
}

void SinkWidget::on_muteButton_toggled(bool checked)
{
    d->sink->setMuted(checked);
}

PulseAudioQt::Sink* SinkWidget::sink() {
    return d->sink;
}

void SinkWidget::on_expandVolumesButton_clicked()
{
    tVariantAnimation* a = new tVariantAnimation();
    a->setStartValue(ui->allVolumesWidget->height());
    if (ui->allVolumesWidget->height() == 0) {
        a->setEndValue(ui->allVolumesWidget->sizeHint().height());
        ui->expandVolumesButton->setIcon(QIcon::fromTheme("go-up"));
    } else {
        a->setEndValue(0);
        ui->expandVolumesButton->setIcon(QIcon::fromTheme("go-down"));
    }
    a->setDuration(500);
    a->setEasingCurve(QEasingCurve::OutCubic);
    connect(a, &tVariantAnimation::valueChanged, a, [=](QVariant value) {
        ui->allVolumesWidget->setFixedHeight(value.toInt());
    });
    connect(a, &tVariantAnimation::finished, a, &tVariantAnimation::deleteLater);
    a->start();
}

void SinkWidget::on_volumeSlider_sliderPressed()
{
    d->movingMaster = true;

    d->masterScales.clear();
    for (int i = 0; i < d->sink->channels().count(); i++) {
        if (ui->volumeSlider->value() == 0) {
            d->masterScales.append(1);
        } else {
            QSlider* slider = static_cast<QSlider*>(ui->volumesLayout->itemAtPosition(i, 1)->widget());
            d->masterScales.append(static_cast<double>(slider->value()) / static_cast<double>(ui->volumeSlider->value()));
        }
    }
}

void SinkWidget::on_volumeSlider_sliderReleased()
{
    d->movingMaster = false;
}

void SinkWidget::on_volumeSlider_valueChanged(int value)
{
//    new tPromise<void>([=](QString error) {
//        if (!d->volumeChangeLocker.tryLock()) return;
//        for (int i = 0; i < d->sink->channels().count(); i++) {
//            int newVol = value * d->masterScales.value(i);
//            d->sink->setChannelVolume(i, newVol);
//            QThread::msleep(20);
//        }
//        d->volumeChangeLocker.unlock();
//    });
    d->sink->setVolume(value);
}

void SinkWidget::on_defaultButton_toggled(bool checked)
{
    if (checked) {
        d->sink->setDefault(true);
    }
}
