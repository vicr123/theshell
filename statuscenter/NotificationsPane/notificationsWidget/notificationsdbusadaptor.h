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

#ifndef NOTIFICATIONSDBUSADAPTOR_H
#define NOTIFICATIONSDBUSADAPTOR_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDBusUnixFileDescriptor>
#include <QDBusMetaType>
#include <QSettings>
#include <QApplication>
#include <QDBusReply>
#include <QSettings>
#include "audiomanager.h"

class NotificationsWidget;

class NotificationsDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.Notifications\">\n"
"    <signal name=\"NotificationClosed\">\n"
"      <arg direction=\"out\" type=\"u\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"u\" name=\"reason\"/>\n"
"    </signal>\n"
"    <signal name=\"ActionInvoked\">\n"
"      <arg direction=\"out\" type=\"u\" name=\"id\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"action_key\"/>\n"
"    </signal>\n"
"    <method name=\"GetCapabilities\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"Notify\">\n"
"      <arg direction=\"out\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app_name\"/>\n"
"      <arg direction=\"in\" type=\"u\" name=\"replaces_id\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app_icon\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"summary\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"body\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"actions\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"hints\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In6\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"expire_timeout\"/>\n"
"    </method>\n"
"    <method name=\"CloseNotification\">\n"
"      <arg direction=\"in\" type=\"u\" name=\"id\"/>\n"
"    </method>\n"
"    <method name=\"GetServerInformation\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"vendor\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"version\"/>\n"
"      <arg direction=\"out\" type=\"s\" name=\"spec_version\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")

public:
    NotificationsDBusAdaptor(QObject* parent = nullptr);
    virtual ~NotificationsDBusAdaptor();

    NotificationsWidget* parentWidget();
    void setParentWidget(NotificationsWidget* parent);

public: // PROPERTIES
public slots: // METHODS
    void CloseNotification(uint id);
    QStringList GetCapabilities();
    QString GetServerInformation(QString &vendor, QString &version, QString &spec_version);
    uint Notify(const QString &app_name, uint replaces_id, const QString &app_icon, const QString &summary, const QString &body, const QStringList &actions, const QVariantMap &hints, int expire_timeout);

signals: // SIGNALS
    void ActionInvoked(uint id, const QString &action_key);
    void NotificationClosed(uint id, uint reason);

private:
    NotificationsWidget* pt = NULL;
    QSettings settings;
};

struct ImageData {
    int width;
    int height;
    int rowstride;
    bool alpha;
    int bitsPerSample;
    int channels;
    QByteArray data;
};
Q_DECLARE_METATYPE(ImageData)

#endif // NOTIFICATIONSDBUSADAPTOR_H
