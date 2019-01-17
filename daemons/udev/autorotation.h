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

#ifndef AUTOROTATION_H
#define AUTOROTATION_H

#include <QVariant>
#include <QObject>
#include <libudev.h>
#include <QTimer>

class Daemon;

class AutoRotation : public QObject
{
    Q_OBJECT
    public:
        explicit AutoRotation(Daemon *parent = nullptr);

        void message(QString name, QVariantList args = QVariantList());

    signals:

    public slots:
        void checkRotation();
        void remapTouchScreens();

    private:
        udev* context;
        QString accelerometerPath;
        QString oldOrientation;
        QString primaryDisplay;

        QTimer* timer;
        Daemon* daemon;

        uint switchId = -1;
};

#endif // AUTOROTATION_H
