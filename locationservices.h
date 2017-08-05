/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef LOCATIONSERVICES_H
#define LOCATIONSERVICES_H

#include <QObject>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>

class LocationServices : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.GeoClue2.Agent")
    Q_PROPERTY(uint MaxAccuracyLevel READ MaxAccuracyLevel)

public:
    explicit LocationServices(QObject *parent = 0);

    uint MaxAccuracyLevel();

public Q_SLOTS:
    Q_SCRIPTABLE bool AuthorizeApp(QString desktop_id, uint req_accuracy_level, uint &allowed_accuracy_level);

signals:
    void locationUsingChanged(bool location);

public slots:
    void GeocluePropertiesChanged(QString interface, QVariantMap properties);

private:

};

#endif // LOCATIONSERVICES_H
