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

SinkInputWidget::SinkInputWidget(pa_context* ctx, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SinkInputWidget)
{
    ui->setupUi(this);

    this->ctx = ctx;
    ui->volumeSlider->setMaximum(PA_VOLUME_NORM);
    ui->volumeSlider->setPageStep(PA_VOLUME_NORM / 20);
}

SinkInputWidget::~SinkInputWidget()
{
    delete ui;
}

int SinkInputWidget::paIndex() {
    return lastIndex;
}

void SinkInputWidget::updateInfo(pa_sink_input_info info) {
    if (info.client == PA_INVALID_INDEX) {
        ui->nameLabel->setText(tr("Playback Stream").toUpper());
    } else {
        pa_context_get_client_info(ctx, info.client, [](pa_context* ctx, const pa_client_info* info, int eol, void* userdata) {

            QPointer<SinkInputWidget>* w = static_cast<QPointer<SinkInputWidget>*>(userdata);
            if (info && !w->isNull()) {
                w->data()->ui->nameLabel->setText(QString::fromLocal8Bit(info->name).toUpper());
            } else if (eol) {
                delete w;
            }
        }, new QPointer<SinkInputWidget>(this));
    }
    ui->descriptionLabel->setText(QString::fromLocal8Bit(info.name));
    ui->muteButton->setChecked(info.mute);

    if (!info.has_volume) {
        ui->volumeSlider->setVisible(false);
    } else {
        ui->volumeSlider->setVisible(true);
        ui->volumeSlider->setEnabled(info.volume_writable);

        double avgVolume = 0;
        for (int i = 0; i < info.volume.channels; i++) {
            avgVolume += info.volume.values[i];
        }
        ui->volumeSlider->setValue(avgVolume / info.volume.channels);
    }

    lastIndex = info.index;
}

void SinkInputWidget::on_muteButton_toggled(bool checked)
{
    emit updateMute(checked, lastIndex);
}
