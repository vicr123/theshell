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
#include "sourcewidget.h"
#include "ui_sourcewidget.h"

#include <Context>
#include <Source>

struct SourceWidgetPrivate {
    PulseAudioQt::Source* source;
};

SourceWidget::SourceWidget(PulseAudioQt::Source* source, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SourceWidget)
{
    ui->setupUi(this);
    d = new SourceWidgetPrivate();

    d->source = source;

    ui->volumeSlider->setMaximum(PulseAudioQt::normalVolume() * 1.5);
    ui->volumeSlider->setPageStep(PulseAudioQt::normalVolume() / 20);

    connect(source, &PulseAudioQt::Source::descriptionChanged, this, [=] {
        ui->nameLabel->setText(source->description().toUpper());
    });
    connect(source, &PulseAudioQt::Source::mutedChanged, this, [=] {
        ui->muteButton->setChecked(source->isMuted());
    });
    connect(source, &PulseAudioQt::Source::volumeChanged, this, [=] {
        ui->volumeSlider->setValue(source->volume());
    });
    connect(source, &PulseAudioQt::Source::defaultChanged, this, [=] {
        ui->defaultButton->setChecked(source->isDefault());
    });

    ui->nameLabel->setText(source->description().toUpper());
    ui->muteButton->setChecked(source->isMuted());
    ui->volumeSlider->setValue(source->volume());
    ui->defaultButton->setChecked(source->isDefault());
}

SourceWidget::~SourceWidget()
{
    delete d;
    delete ui;
}

PulseAudioQt::Source*SourceWidget::source()
{
    return d->source;
}

void SourceWidget::on_muteButton_toggled(bool checked)
{
    d->source->setMuted(checked);
}

void SourceWidget::on_volumeSlider_valueChanged(int value)
{
    d->source->setVolume(value);
}

void SourceWidget::on_defaultButton_toggled(bool checked)
{
    d->source->setDefault(checked);
}
