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
#include "notificationspermissionengine.h"

#include "qsettingsformats.h"
#include <QSettings>
#include <QCryptographicHash>
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QLocale>
#include "application.h"

struct NotificationsPermissionEnginePrivate {
    QSettings* appSettings;
    QScopedPointer<Application> desktopApp;
    bool isValidApp = true;
    bool istheshell = false;
};

NotificationsPermissionEngine::NotificationsPermissionEngine(QString appName, QString desktopFile)
{
    d = new NotificationsPermissionEnginePrivate();
    d->desktopApp.reset(new Application());
    d->isValidApp = true;
    d->appSettings = new QSettings("theSuite", "theShell-notifications");

    //Determine the name of the group used to store these settings
    QString groupName;
    if (desktopFile != "") {
        //Search for desktop file
        d->desktopApp.reset(new Application(desktopFile));

        if (d->desktopApp->isValid()) {
            //Desktop file was found, use desktop settings
            groupName = "dsk-" + desktopFile;
        } else {
            //Desktop file not found, use app name settings
            groupName = "app-" + appName;
        }
    } else {
        groupName = "app-" + appName;
        if (appName == "theShell") {
            d->istheshell = true;
        }
    }

    //Check if notification settings for this app exist
    if (!d->appSettings->childGroups().contains(groupName)) {
        //Notification settings don't exist, create them now
        d->appSettings->beginGroup(groupName);
        if (!d->desktopApp->isValid()) {
            d->appSettings->setValue("isDesktopFile", false);
            d->appSettings->setValue("identifier", appName);
        } else {
            d->appSettings->setValue("isDesktopFile", true);
            d->appSettings->setValue("identifier", desktopFile);
        }
    } else {
        //Notification settings exist
        d->appSettings->beginGroup(groupName);
    }
}

NotificationsPermissionEngine::NotificationsPermissionEngine() {
    d = new NotificationsPermissionEnginePrivate();
    d->isValidApp = false;
    d->desktopApp.reset(new Application());
}

NotificationsPermissionEngine::~NotificationsPermissionEngine() {
    d->appSettings->endGroup();
    d->appSettings->deleteLater();
    delete d;
}

QStringList NotificationsPermissionEngine::knownApps() {
    QStringList returnValue;
    QSettings appSettings("theSuite", "theShell-notifications");
    for (QString group : appSettings.childGroups()) {
        appSettings.beginGroup(group);
        if (appSettings.contains("identifier") && appSettings.value("isDesktopFile").toBool() == false && appSettings.value("identifier").toString() != "theShell") {
            returnValue.append(appSettings.value("identifier").toString());
        }
        appSettings.endGroup();
    }
    return returnValue;
}

QStringList NotificationsPermissionEngine::knownDesktopFiles() {
    QStringList returnValue;
    QSettings appSettings("theSuite", "theShell-notifications");
    for (QString group : appSettings.childGroups()) {
        appSettings.beginGroup(group);
        if (appSettings.contains("identifier") && appSettings.value("isDesktopFile").toBool() == true) {
            returnValue.append(appSettings.value("identifier").toString());
        }
        appSettings.endGroup();
    }
    return returnValue;
}

bool NotificationsPermissionEngine::isValidApp() {
    return d->isValidApp;
}

bool NotificationsPermissionEngine::isDesktopFile() {
    if (!d->isValidApp) return false;
    return d->appSettings->value("isDesktopFile", false).toBool();
}

QString NotificationsPermissionEngine::identifier() {
    if (!d->isValidApp) return "";
    return d->appSettings->value("identifier").toString();
}

QIcon NotificationsPermissionEngine::appIcon() {
    if (!d->isValidApp) return QIcon::fromTheme("generic-app");
    if (d->istheshell) return QIcon::fromTheme("theshell");
    if (!d->desktopApp->isValid()) {
        if (QIcon::hasThemeIcon(identifier().toLower().replace(" ", "-"))) {
           return QIcon::fromTheme(identifier().toLower().replace(" ", "-"));
        } else if (QIcon::hasThemeIcon(identifier().toLower().replace(" ", ""))) {
           return QIcon::fromTheme(identifier().toLower().replace(" ", ""));
        } else {
            return QIcon::fromTheme("generic-app");
        }
    } else {
         QString iconName = d->desktopApp->getProperty("Icon").toString();
         if (QFile(iconName).exists()) {
             return QIcon(iconName);
         } else {
             return QIcon::fromTheme(iconName);
         }
    }
}

QString NotificationsPermissionEngine::appName() {
    if (!d->isValidApp) return "";
    if (!d->desktopApp->isValid()) {
        return identifier();
    } else {
        return d->desktopApp->getProperty("Name").toString();
    }
}

void NotificationsPermissionEngine::remove() {
    //Clear this app's settings
    d->appSettings->remove("");
    d->isValidApp = false;
}

void NotificationsPermissionEngine::setAllowNotifications(bool allowNotifications) {
    if (!d->isValidApp || d->istheshell) return;
    d->appSettings->setValue("allow", allowNotifications);
}

bool NotificationsPermissionEngine::allowNotifications() {
    if (!d->isValidApp) return false;
    return d->appSettings->value("allow", true).toBool();
}

void NotificationsPermissionEngine::setBypassesQuietMode(bool bypassesQuietMode) {
    if (!d->isValidApp || d->istheshell) return;
    d->appSettings->setValue("bypassQuiet", bypassesQuietMode);
}

bool NotificationsPermissionEngine::bypassesQuietMode() {
    if (!d->isValidApp) return false;
    return d->appSettings->value("bypassQuiet", false).toBool();
}

void NotificationsPermissionEngine::setShowPopups(bool popups) {
    if (!d->isValidApp || d->istheshell) return;
    d->appSettings->setValue("popup", popups);
}

bool NotificationsPermissionEngine::showPopups() {
    if (!d->isValidApp) return false;
    return d->appSettings->value("popup", true).toBool();
}

void NotificationsPermissionEngine::setPlaySound(bool playSound) {
    if (!d->isValidApp || d->istheshell) return;
    d->appSettings->setValue("sounds", playSound);
}

bool NotificationsPermissionEngine::playSound() {
    if (!d->isValidApp) return false;
    return d->appSettings->value("sounds", true).toBool();
}
