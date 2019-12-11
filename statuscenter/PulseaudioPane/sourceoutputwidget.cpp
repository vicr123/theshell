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
#include "sourceoutputwidget.h"
#include "ui_sourceoutputwidget.h"

#include <Context>
#include <Source>
#include <SourceOutput>
#include "deviceselection.h"
#include <tpopover.h>

struct SourceOutputWidgetPrivate {
    PulseAudioQt::SourceOutput* output;
};

SourceOutputWidget::SourceOutputWidget(PulseAudioQt::SourceOutput* sourceOutput, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SourceOutputWidget)
{
    ui->setupUi(this);
    d = new SourceOutputWidgetPrivate();

    d->output = sourceOutput;
    ui->volumeSlider->setMaximum(static_cast<int>(PulseAudioQt::normalVolume()));
    ui->volumeSlider->setPageStep(static_cast<int>(PulseAudioQt::normalVolume() / 20));

    connect(sourceOutput, &PulseAudioQt::SourceOutput::nameChanged, this, &SourceOutputWidget::updateName);
    connect(sourceOutput, &PulseAudioQt::SourceOutput::clientChanged, this, [=] {
        connect(sourceOutput->client(), &PulseAudioQt::Client::nameChanged, this, &SourceOutputWidget::updateName);
        updateName();
    });
    connect(sourceOutput, &PulseAudioQt::SourceOutput::mutedChanged, this, [=] {
        ui->muteButton->setChecked(sourceOutput->isMuted());
        emit listeningStateChanged();
    });
    connect(sourceOutput, &PulseAudioQt::SourceOutput::hasVolumeChanged, this, [=] {
        ui->volumeSlider->setVisible(sourceOutput->hasVolume());
    });
    connect(sourceOutput, &PulseAudioQt::SourceOutput::isVolumeWritableChanged, this, [=] {
        ui->volumeSlider->setEnabled(sourceOutput->isVolumeWritable());
    });
    connect(sourceOutput, &PulseAudioQt::SourceOutput::deviceIndexChanged, this, [=] {
        for (PulseAudioQt::Source* source : PulseAudioQt::Context::instance()->sources()) {
            if (source->index() == sourceOutput->deviceIndex()) {
                ui->sourceSelectionButton->setText(source->description());
            }
        }
    });
    connect(sourceOutput, &PulseAudioQt::SourceOutput::volumeChanged, this, [=] {
        ui->volumeSlider->setValue(static_cast<int>(d->output->volume()));
    });

    ui->volumeSlider->setVisible(sourceOutput->hasVolume());
    ui->volumeSlider->setEnabled(sourceOutput->isVolumeWritable());
    ui->descriptionLabel->setText(sourceOutput->name());
    ui->muteButton->setChecked(sourceOutput->isMuted());
    ui->volumeSlider->setValue(static_cast<int>(d->output->volume()));
    for (PulseAudioQt::Source* source : PulseAudioQt::Context::instance()->sources()) {
        if (source->index() == sourceOutput->deviceIndex()) {
            ui->sourceSelectionButton->setText(source->description());
        }
    }
    updateName();
}

SourceOutputWidget::~SourceOutputWidget()
{
    delete d;
    delete ui;
}

PulseAudioQt::SourceOutput*SourceOutputWidget::sourceOutput()
{
    return d->output;
}

SourceOutputWidget::ListeningState SourceOutputWidget::listeningState()
{
    if (d->output->isMuted()) return BlockedFromListening;
    return Listening;
}

void SourceOutputWidget::updateName()
{
    if (d->output->client()) {
        ui->nameLabel->setText(d->output->name());
        ui->descriptionLabel->setText(d->output->client()->name());
    } else {
        ui->nameLabel->setText(tr("Playback").toUpper());
        ui->descriptionLabel->setText(d->output->name());
    }
}

void SourceOutputWidget::on_sourceSelectionButton_clicked()
{
    DeviceSelection* selection = new DeviceSelection(DeviceSelection::Source);
    tPopover* popover = new tPopover(selection);
    popover->setPopoverWidth(SC_DPI(400));
    connect(selection, &DeviceSelection::rejected, popover, &tPopover::dismiss);
    connect(selection, &DeviceSelection::accepted, this, [=](quint32 index) {
        d->output->setDeviceIndex(index);
        popover->dismiss();
    });
    connect(popover, &tPopover::dismissed, selection, &DeviceSelection::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    popover->show(this->window());
}

void SourceOutputWidget::on_muteButton_toggled(bool checked)
{
    d->output->setMuted(checked);
}
