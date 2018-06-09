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

#include "dbussignals.h"
#include "theshell_adaptor.h"
#include "mainwindow.h"

extern MainWindow* MainWin;

DBusSignals::DBusSignals(QObject *parent) : QObject(parent)
{
    new TheshellAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/thesuite/theshell", this);
    dbus.registerService("org.thesuite.theshell");
}

void DBusSignals::NextKeyboard() {
    MainWin->getInfoPane()->setNextKeyboardLayout();
    //Hotkeys->show(QIcon::fromTheme("input-keyboard"), tr("Keyboard Layout"), tr("Keyboard Layout set to %1").arg(newKeyboardLayout), 5000);
}
