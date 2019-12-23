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
#include "localemanager.h"

#include <QTranslator>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QLibraryInfo>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusObjectPath>
#include <unistd.h>

struct LocaleManagerPrivate {
    LocaleManager* instance = nullptr;

    QTranslator* qtTranslator = nullptr;
    QTranslator* appTranslator = nullptr;

    QList<QLocale> locales;
};

LocaleManagerPrivate* LocaleManager::d = new LocaleManagerPrivate();

LocaleManager::LocaleManager() : QObject(nullptr) {

}

void LocaleManager::writeLocaleSettings()
{
    QSettings settings;
    settings.beginWriteArray("locale/lang", d->locales.count());
    for (int i = 0; i < d->locales.count(); i++) {
        settings.setArrayIndex(i);
        settings.setValue("bcp47", d->locales.at(i).bcp47Name());
    }
    settings.endArray();

    //Talk to AccountsService to set the user's locale
    QDBusMessage findUserMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "FindUserById");
    findUserMessage.setArguments({static_cast<qlonglong>(geteuid())});
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(findUserMessage));
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        if (!watcher->isError()) {
            QDBusObjectPath path = watcher->reply().arguments().first().value<QDBusObjectPath>();

            QDBusMessage setMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", path.path(), "org.freedesktop.Accounts.User", "SetLanguage");
            setMessage.setArguments({d->locales.first().bcp47Name().replace("-", "_")});
            QDBusConnection::systemBus().asyncCall(setMessage);
        }
    });

    updateLocaleSettings();
}

void LocaleManager::initialize()
{
    d->instance = new LocaleManager();

    d->qtTranslator = new QTranslator();
    d->appTranslator = new QTranslator();

    QSettings settings;
    int count = settings.beginReadArray("locale/lang");
    if (count == 0) {
        d->locales.append(QLocale(QLocale::English, QLocale::UnitedStates));
    } else {
        for (int i = 0; i < count; i++) {
            settings.setArrayIndex(i);
            d->locales.append(QLocale(settings.value("bcp47").toString()));
        }
    }
    settings.endArray();

    LocaleManager::updateLocaleSettings();

    QApplication::installTranslator(d->qtTranslator);
    QApplication::installTranslator(d->appTranslator);
}

LocaleManager*LocaleManager::instance()
{
    return d->instance;
}

QList<QLocale> LocaleManager::availableLanguages()
{
    QDir translationsDir("/usr/share/theshell/translations");
    QList<QLocale> locales;
    for (QFileInfo info : translationsDir.entryInfoList()) {
        if (info.suffix() == "qm") {
            locales.append(QLocale(info.baseName()));
        }
    }
    return locales;
}

QList<QLocale> LocaleManager::currentLocale()
{
    return d->locales;
}

void LocaleManager::addLocale(QLocale locale)
{
    d->locales.append(locale);
    writeLocaleSettings();
}

void LocaleManager::removeLocale(QLocale locale)
{
    if (d->locales.count() == 1) return; //Don't empty the locale list
    d->locales.removeOne(locale);
    writeLocaleSettings();
}

void LocaleManager::moveLocaleUp(QLocale locale)
{
    int idx = d->locales.indexOf(locale);
    if (idx == 0) return; //Can't move this locale up
    d->locales.takeAt(idx);
    d->locales.insert(idx - 1, locale);
    writeLocaleSettings();
}

void LocaleManager::moveLocaleDown(QLocale locale)
{
    int idx = d->locales.indexOf(locale);
    if (idx == d->locales.count() - 1) return; //Can't move this locale down
    d->locales.takeAt(idx);
    d->locales.insert(idx + 1, locale);
    writeLocaleSettings();
}

void LocaleManager::updateLocaleSettings()
{
    qputenv("LANG", d->locales.first().bcp47Name().toUtf8());

    QStringList languages;
    for (QLocale locale : d->locales) {
        languages.append(locale.bcp47Name());
    }
    qputenv("LANGUAGE", languages.join(":").toUtf8());

    QLocale::setDefault(d->locales.first());
    QApplication::setLayoutDirection(d->locales.first().textDirection());

    d->qtTranslator->load("qt_" + d->locales.first().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));

//    if (locale.name() == "C") {
//        d->tsTranslator->load(localeName, QString(SHAREDIR) + "translations");
//    } else {
        d->appTranslator->load(d->locales.first().name(), "/usr/share/theshell/translations");
//    }

    //Tell all plugins to update translator
    emit d->instance->localeChanged();

    //Process all events
    QApplication::processEvents();
}
