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
#include "gesturepane.h"
#include "ui_gesturepane.h"

#include <QSettings>

struct GesturePanePrivate {
    QSettings settings;
};

GesturePane::GesturePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GesturePane)
{
    ui->setupUi(this);
    d = new GesturePanePrivate();

    ui->touchModeSwitch->setChecked(d->settings.value("input/touch", false).toBool());
}

GesturePane::~GesturePane()
{
    delete ui;
    delete d;
}

void GesturePane::on_touchModeSwitch_toggled(bool checked)
{
    d->settings.setValue("input/touch", checked);
}
