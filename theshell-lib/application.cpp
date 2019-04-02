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
#include "application.h"

#include "qsettingsformats.h"
#include <QDir>
#include <QDirIterator>
#include <QLocale>
#include <QSet>
#include <QFileSystemWatcher>


struct ApplicationPrivate {
    QSettings* appSettings = nullptr;
    QVariantMap details;

    bool useSettings = false;
    QString desktopEntry;
};

const QStringList searchPaths = {
    QDir::homePath() + "/.local/share/applications",
    "/usr/share/applications"
};
ApplicationDaemon* ApplicationDaemon::d = nullptr;

Application::Application(QString desktopEntry) {
    d = new ApplicationPrivate();

    for (QString searchPath : searchPaths) {
        QDirIterator iterator(searchPath, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            if (desktopEntry == iterator.fileInfo().completeBaseName()) {
                //We found the file
                d->appSettings = new QSettings(iterator.filePath(), QSettingsFormats::desktopFormat());
                d->desktopEntry = desktopEntry;
                d->useSettings = true;
                return;
            }
        }
    }
}

Application::Application(QVariantMap details) {
    d = new ApplicationPrivate();

    d->details = details;
    d->useSettings = false;
}

Application::~Application() {
    delete d;
}

bool Application::hasProperty(QString propertyName) const {
    QLocale locale;
    if (d->useSettings) d->appSettings->beginGroup("Desktop Entry");

    //Check to see if there's a localised version of the property name
    QString usedKey = propertyName;
    for (QString trialKey : {"[" + locale.name() + "]", "[" + locale.name().split("_").first() + "]", QString("")}) {
        QString test = propertyName + trialKey;
        if ((d->useSettings && d->appSettings->contains(test)) || (!d->useSettings && d->details.contains(test))) {
            return true;
        }
    }
    return false;
}

QVariant Application::getProperty(QString propertyName, QVariant defaultValue) const {
    QLocale locale;
    if (d->useSettings) d->appSettings->beginGroup("Desktop Entry");

    //Check to see if there's a localised version of the property name
    QString usedKey = propertyName;
    for (QString trialKey : {"[" + locale.name() + "]", "[" + locale.name().split("_").first() + "]"}) {
        QString test = propertyName + trialKey;
        if ((d->useSettings && d->appSettings->contains(test)) || (!d->useSettings && d->details.contains(test))) {
            usedKey = test;
            break;
        }
    }

    //Return the property
    QVariant retval = d->useSettings ? d->appSettings->value(usedKey, defaultValue) : d->details.value(usedKey, defaultValue);
    if (d->useSettings) d->appSettings->endGroup();
    return retval;
}

QVariant Application::getActionProperty(QString action, QString propertyName, QVariant defaultValue) const {
    QLocale locale;
    if (d->useSettings) d->appSettings->beginGroup("Desktop Action " + action);

    //Check to see if there's a localised version of the property name
    QString usedKey = propertyName;
    for (QString trialKey : {"[" + locale.name() + "]", "[" + locale.name().split("_").first() + "]"}) {
        QString test = propertyName + trialKey;
        if ((d->useSettings && d->appSettings->contains(test)) || (!d->useSettings && d->details.contains(test))) {
            usedKey = test;
            break;
        }
    }

    //Return the property
    QVariant retval = d->useSettings ? d->appSettings->value(usedKey, defaultValue) : d->details.value(usedKey, defaultValue);
    if (d->useSettings) d->appSettings->endGroup();
    return retval;
}

QStringList Application::getStringList(QString propertyName, QStringList defaultValue) const {
    QString property = getProperty(propertyName, defaultValue).toString();
    return property.split(";", QString::SkipEmptyParts);
}

QStringList Application::allApplications() {
    QSet<QString> stringSet;
    for (QString searchPath : searchPaths) {
        QDirIterator iterator(searchPath, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            stringSet.insert(iterator.fileInfo().completeBaseName());
        }
    }
    return stringSet.toList();
}

QString Application::desktopEntry() const {
    return d->desktopEntry;
}

#include <QDebug>
ApplicationDaemon::ApplicationDaemon() : QObject(nullptr) {
    QFileSystemWatcher* watcher = new QFileSystemWatcher();
    watcher->addPaths(searchPaths);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &ApplicationDaemon::appsUpdateRequired);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &ApplicationDaemon::appsUpdateRequired);
}

ApplicationDaemon* ApplicationDaemon::instance() {
    if (d == nullptr) d = new ApplicationDaemon();
    return d;
}
