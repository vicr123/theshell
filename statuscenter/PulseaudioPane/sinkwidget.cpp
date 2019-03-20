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
#include <pulse/channelmap.h>
#include "maps.h"

struct SinkWidgetPrivate {
    int lastIndex = 0;
    int lastChannels = 0;
    int masterTracking = 0;

    bool movingMaster = false;
    QList<double> masterScales;

    QString sinkName;
};

SinkWidget::SinkWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SinkWidget)
{
    ui->setupUi(this);

    d = new SinkWidgetPrivate;

    ui->volumeSlider->setMaximum(PA_VOLUME_NORM * 1.5);
    ui->volumeSlider->setPageStep(PA_VOLUME_NORM / 20);

    ui->allVolumesWidget->setFixedHeight(0);
}

SinkWidget::~SinkWidget()
{
    delete d;
    delete ui;
}

void SinkWidget::updateInfo(pa_sink_info info, QString defaultSinkName) {
    ui->deviceName->setText(QString::fromLocal8Bit(info.description).toUpper());
    ui->muteButton->setChecked(info.mute);

    if (d->lastChannels != info.channel_map.channels) {
        QLayoutItem* item;
        while ((item = ui->volumesLayout->takeAt(0))) {
            ui->volumesLayout->removeItem(item);
            item->widget()->deleteLater();
        }

        for (int i = 0; i < info.channel_map.channels; i++) {
            int speakerIndex = i;
            QString text = tr("Speaker");
            if (channelPositionToString.contains(info.channel_map.map[speakerIndex])) {
                text = QApplication::translate("maps", channelPositionToString.value(info.channel_map.map[speakerIndex]));
            }

            QLabel* label = new QLabel();
            label->setText(text);
            label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            ui->volumesLayout->addWidget(label, speakerIndex, 0);

            QSlider* slider = new QSlider();
            slider->setMaximum(PA_VOLUME_NORM * 1.5);
            slider->setPageStep(PA_VOLUME_NORM / 20);
            slider->setOrientation(Qt::Horizontal);
            slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            connect(slider, &QSlider::valueChanged, slider, [=](int value) {
                pa_cvolume newVolume;
                newVolume.channels = d->lastChannels;
                for (int i = 0; i < d->lastChannels; i++) {
                    QSlider* slider = static_cast<QSlider*>(ui->volumesLayout->itemAtPosition(i, 1)->widget());
                    newVolume.values[i] = slider->value();
                }

                emit updateVolume(newVolume, d->lastIndex);
            });
            ui->volumesLayout->addWidget(slider, speakerIndex, 1);
        }

        if (ui->allVolumesWidget->height() != 0) ui->allVolumesWidget->setFixedHeight(ui->allVolumesWidget->sizeHint().height());
        d->lastChannels = info.channel_map.channels;
    }

    int maxVolumeIndex = 0;
    for (int i = 0; i < info.volume.channels; i++) {
        QSlider* slider = static_cast<QSlider*>(ui->volumesLayout->itemAtPosition(i, 1)->widget());
        QSignalBlocker blocker(slider);
        slider->setValue(info.volume.values[i]);

        if (info.volume.values[maxVolumeIndex] < info.volume.values[i]) {
            maxVolumeIndex = i;
        }
    }

    if (!d->movingMaster) {
        QSignalBlocker blocker(ui->volumeSlider);
        ui->volumeSlider->setValue(info.volume.values[maxVolumeIndex]);
        d->masterTracking = maxVolumeIndex;
    }

    d->lastIndex = info.index;
    d->sinkName = QString::fromLocal8Bit(info.name);

    defaultSinkChanged(defaultSinkName);
}

void SinkWidget::defaultSinkChanged(QString defaultSinkName) {
    ui->defaultButton->setChecked(d->sinkName == defaultSinkName);
}

void SinkWidget::on_muteButton_toggled(bool checked)
{
    emit updateMute(checked, d->lastIndex);
}

int SinkWidget::paIndex() {
    return d->lastIndex;
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
    for (int i = 0; i < d->lastChannels; i++) {
        if (ui->volumeSlider->value() == 0) {
            d->masterScales.append(1);
        } else {
            QSlider* slider = static_cast<QSlider*>(ui->volumesLayout->itemAtPosition(i, 1)->widget());
            d->masterScales.append((double) slider->value() / (double) ui->volumeSlider->value());
        }
    }
}

void SinkWidget::on_volumeSlider_sliderReleased()
{
    d->movingMaster = false;
}

void SinkWidget::on_volumeSlider_valueChanged(int value)
{
    pa_cvolume newVolume;
    newVolume.channels = d->masterScales.count();
    for (int i = 0; i < d->masterScales.count(); i++) {
        newVolume.values[i] = value * d->masterScales.value(i);
    }

    emit updateVolume(newVolume, d->lastIndex);
}

void SinkWidget::on_defaultButton_toggled(bool checked)
{
    if (checked) {
        emit setDefault(d->sinkName);
    }
}
