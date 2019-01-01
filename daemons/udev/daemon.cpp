/****************************************
 *
 *   theShell - Desktop Environment
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

#include "daemon.h"

Daemon::Daemon(QObject *parent) : QObject(parent)
{
    rotation = new AutoRotation(this);
}

QWidget* Daemon::mainWidget() {
    return nullptr;
}


QString Daemon::name() {
    return "udev";
}

StatusCenterPaneObject::StatusPaneTypes Daemon::type() {
    return None;
}

int Daemon::position() {
    return 0;
}

void Daemon::message(QString name, QVariantList args) {
    rotation->message(name, args);
}
