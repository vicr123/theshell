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
#include "localemodel.h"

#include "localemanager.h"

struct LocaleModelPrivate {
    QList<QLocale::Language> locales;
};

LocaleModel::LocaleModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new LocaleModelPrivate();
    for (int i = QLocale::C + 1; i < QLocale::LastLanguage; i++) {
        if (i == QLocale::UncodedLanguages) continue;
        QLocale locale(static_cast<QLocale::Language>(i));
        if (locale.language() != i) continue;
        if (!locale.nativeLanguageName().isEmpty()) {
            d->locales.append(locale.language());
        }
    }
    std::sort(d->locales.begin(), d->locales.end(), [](const QLocale::Language& a, const QLocale::Language& b) {
        return textForLanguage(a).localeAwareCompare(textForLanguage(b)) < 0;
    });
}

LocaleModel::~LocaleModel()
{
    delete d;
}

int LocaleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return d->locales.count();
}

QVariant LocaleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    QLocale::Language lang = d->locales.at(index.row());
    QLocale locale(lang);
    switch (role) {
        case Qt::DisplayRole:
            return textForLanguage(lang);
    }

    return QVariant();
}

QModelIndex LocaleModel::indexOf(QLocale::Language language)
{
    return index(d->locales.indexOf(language));
}

QLocale::Language LocaleModel::language(QModelIndex index)
{
    return d->locales.at(index.row());
}

QString LocaleModel::textForLanguage(QLocale::Language language)
{
    QLocale locale(language);
    QString name;
    if (language == QLocale::English) {
        return "English"; //Do not localise
    } else {
        QString native = locale.nativeLanguageName();
        if (native.isEmpty()) {
            return QLocale::languageToString(language);
        } else {
            return QStringLiteral("%1 (%2)").arg(locale.nativeLanguageName()).arg(QLocale::languageToString(language));
        }
    }
}
