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

#include "ui_displayconfigurationwidget.h"
#include "displayconfigurationwidget.h"

struct DisplayConfigurationWidgetPrivate {
    QList<XRRModeInfo> modes;
};

DisplayConfigurationWidget::DisplayConfigurationWidget(QString displayName, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DisplayConfigurationWidget)
{
    ui->setupUi(this);
    d = new DisplayConfigurationWidgetPrivate();

    ui->displayName->setText(displayName);

    this->setFixedWidth(300);
    this->setFixedHeight(this->sizeHint().height());
}

DisplayConfigurationWidget::~DisplayConfigurationWidget()
{
    delete d;
    delete ui;
}

void DisplayConfigurationWidget::setModes(QList<XRRModeInfo> modes) {
    ui->resBox->clear();
    QList<QSize> displayed;
    for (XRRModeInfo mode : modes) {
        QSize resolution(mode.width, mode.height);
        if (!displayed.contains(resolution)) {
            ui->resBox->addItem(QString("%1 × %2").arg(resolution.width()).arg(resolution.height()), resolution);
            displayed.append(resolution);
        }
    }

    d->modes = modes;
}

void DisplayConfigurationWidget::setCurrentMode(XRRModeInfo mode) {
    QSize resolution(mode.width, mode.height);
    for (int i = 0; i < ui->resBox->count(); i++) {
        if (ui->resBox->itemData(i).toSize() == resolution) {
            ui->resBox->setCurrentIndex(i);
            on_resBox_currentIndexChanged(i);
        }
    }
}

void DisplayConfigurationWidget::setPowered(bool powered) {
    ui->displayPower->setChecked(powered);
}

void DisplayConfigurationWidget::on_resBox_currentIndexChanged(int index)
{
    ui->framerateBox->clear();
    QSize resolution = ui->resBox->itemData(index).toSize();
    emit resolutionChanged(resolution);

    for (XRRModeInfo aMode : d->modes) {
        if (QSize(aMode.width, aMode.height) == resolution) {
            int vTotal = aMode.vTotal;
            if (aMode.modeFlags & RR_DoubleScan) vTotal *= 2;
            if (aMode.modeFlags & RR_Interlace) vTotal /= 2;
            double framerate = ((double) aMode.dotClock / ((double) aMode.hTotal * (double) vTotal));

            ui->framerateBox->addItem(QString("%1 hz").append(aMode.modeFlags & RR_Interlace ? "/i" : "").arg(framerate, 0, 'f', 2), QVariant::fromValue(aMode.id));
        }
    }
}

RRMode DisplayConfigurationWidget::mode() {
    return ui->framerateBox->currentData().value<RRMode>();
}

bool DisplayConfigurationWidget::powered() {
    return ui->displayPower->isChecked();
}

void DisplayConfigurationWidget::on_displayPower_toggled(bool checked)
{
    emit poweredChanged(checked);
    ui->defaultButton->setEnabled(checked);
}

void DisplayConfigurationWidget::on_defaultButton_toggled(bool checked)
{
    if (checked) {
        emit setDefault();
    }
}

void DisplayConfigurationWidget::setIsDefault(bool isDefault) {
    ui->defaultButton->blockSignals(true);
    ui->defaultButton->setChecked(isDefault);
    ui->defaultButton->blockSignals(false);
}
