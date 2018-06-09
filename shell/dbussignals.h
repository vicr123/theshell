/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#ifndef DBUSSIGNALS_H
#define DBUSSIGNALS_H

#include <QObject>
#include <QDBusConnection>

class DBusSignals : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.theshell")

    public:
        explicit DBusSignals(QObject *parent = 0);

    signals:
        Q_SCRIPTABLE void ThemeChanged();
        Q_SCRIPTABLE void Ready();
        Q_SCRIPTABLE void ShowSplash();
        Q_SCRIPTABLE void HideSplash();

    public Q_SLOTS:
        Q_SCRIPTABLE void NextKeyboard();
};

#endif // DBUSSIGNALS_H
