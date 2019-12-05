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

#include "plugin.h"
#include "powerpane.h"

#include <QDBusConnection>
#include <QDBusMessage>

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    translator = new QTranslator;

    //Inhibit logind's handling of some power events
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Inhibit");
    message.setArguments(QList<QVariant>() << "handle-power-key:handle-suspend-key:handle-lid-switch" << "theShell" << "theShell Handles Hardware Power Keys" << "block");
    QDBusMessage inhibitReply = QDBusConnection::systemBus().call(message);
    this->logindInhibitor = inhibitReply.arguments().first().value<QDBusUnixFileDescriptor>();

    panes.append(new PowerPane);
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {
    translator->load(language, QString(SHAREDIR) + "translations");
    QApplication::instance()->installTranslator(translator);
}
