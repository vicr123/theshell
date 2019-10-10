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
#include "sinkinputwidget.h"
#include "ui_sinkinputwidget.h"

#include <QPointer>

#include <Context>
#include <SinkInput>

struct SinkInputWidgetPrivate {
    PulseAudioQt::SinkInput* input;
    int lastIndex = 0;
};

SinkInputWidget::SinkInputWidget(PulseAudioQt::SinkInput* sinkInput, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SinkInputWidget)
{
    ui->setupUi(this);

    d = new SinkInputWidgetPrivate();

    d->input = sinkInput;
    ui->volumeSlider->setMaximum(PulseAudioQt::normalVolume());
    ui->volumeSlider->setPageStep(PulseAudioQt::normalVolume() / 20);

    connect(sinkInput, &PulseAudioQt::SinkInput::nameChanged, this, &SinkInputWidget::updateName);
    connect(sinkInput, &PulseAudioQt::SinkInput::clientChanged, this, [=] {
        connect(sinkInput->client(), &PulseAudioQt::Client::nameChanged, this, &SinkInputWidget::updateName);
        updateName();
    });
    connect(sinkInput, &PulseAudioQt::SinkInput::mutedChanged, this, [=] {
        ui->muteButton->setChecked(sinkInput->isMuted());
    });
    connect(sinkInput, &PulseAudioQt::SinkInput::hasVolumeChanged, this, [=] {
        ui->volumeSlider->setVisible(sinkInput->hasVolume());
    });
    connect(sinkInput, &PulseAudioQt::SinkInput::isVolumeWritableChanged, this, [=] {
        ui->volumeSlider->setEnabled(sinkInput->isVolumeWritable());
    });
    connect(sinkInput, &PulseAudioQt::SinkInput::volumeChanged, this, &SinkInputWidget::updateVolume);

    ui->volumeSlider->setVisible(sinkInput->hasVolume());
    ui->volumeSlider->setEnabled(sinkInput->isVolumeWritable());
    ui->descriptionLabel->setText(sinkInput->name());
    ui->muteButton->setChecked(sinkInput->isMuted());
    updateName();
    updateVolume();
}

SinkInputWidget::~SinkInputWidget()
{
    delete d;
    delete ui;
}

PulseAudioQt::SinkInput* SinkInputWidget::sinkInput() {
    return d->input;
}

void SinkInputWidget::on_muteButton_toggled(bool checked)
{
    d->input->setMuted(checked);
}

void SinkInputWidget::updateName()
{
    if (d->input->client()) {
        ui->nameLabel->setText(d->input->name());
        ui->descriptionLabel->setText(d->input->client()->name());
    } else {
        ui->nameLabel->setText(tr("Playback").toUpper());
        ui->descriptionLabel->setText(d->input->name());
    }
}

void SinkInputWidget::updateVolume()
{
    ui->volumeSlider->setValue(d->input->volume());
}

void SinkInputWidget::on_volumeSlider_sliderMoved(int position)
{
    d->input->setVolume(position);
}
