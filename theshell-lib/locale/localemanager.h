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
#ifndef LOCALEMANAGER_H
#define LOCALEMANAGER_H

#include <QLocale>

struct LocaleManagerPrivate;

class LocaleManager : public QObject
{
        Q_OBJECT
    public:
        static void initialize();
        static LocaleManager* instance();

        static QList<QLocale> availableLanguages();
        static QList<QLocale> currentLocale();

        static void addLocale(QLocale locale);
        static void removeLocale(QLocale locale);
        static void moveLocaleUp(QLocale locale);
        static void moveLocaleDown(QLocale locale);

    signals:
        void localeChanged();

    private:
        explicit LocaleManager();
        static LocaleManagerPrivate* d;

        static void writeLocaleSettings();
        static void updateLocaleSettings();
};

#endif // LOCALEMANAGER_H
