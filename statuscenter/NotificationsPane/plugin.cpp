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
#include "notificationsWidget/notificationsdbusadaptor.h"
#include "notificationsWidget/notificationswidget.h"

#include "audiomanager.h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    translator = new QTranslator;

    panes.append(AudioManager::instance());

    QObject* notificationParent = new QObject();
    NotificationsDBusAdaptor* adaptor = new NotificationsDBusAdaptor(notificationParent);

    QDBusConnection::sessionBus().registerObject("/org/freedesktop/Notifications", "org.freedesktop.Notifications", notificationParent);
    QDBusConnection::sessionBus().registerService("org.freedesktop.Notifications");

    NotificationsWidget* widget = new NotificationsWidget();
    panes.append(widget);
    adaptor->setParentWidget(widget);
    widget->setAdaptor(adaptor);
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {
    translator->load(language, QString(SHAREDIR) + "translations");
    QApplication::instance()->installTranslator(translator);
}
