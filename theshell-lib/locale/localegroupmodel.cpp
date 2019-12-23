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
#include "localegroupmodel.h"

struct LocaleGroupModelPrivate {
    QLocale::Language language;
};

LocaleGroupModel::LocaleGroupModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new LocaleGroupModelPrivate();
}

LocaleGroupModel::~LocaleGroupModel()
{
    delete d;
}

int LocaleGroupModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return QLocale::countriesForLanguage(d->language).count();
}

QVariant LocaleGroupModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    QLocale::Country ctr = QLocale::countriesForLanguage(d->language).at(index.row());
    QLocale locale(d->language, ctr);
    switch (role) {
        case Qt::DisplayRole:
            return QStringLiteral("%1 (%2)").arg(QLocale::countryToString(ctr)).arg(locale.nativeCountryName());
    }

    return QVariant();
}

void LocaleGroupModel::setLanguage(QLocale::Language language)
{
    d->language = language;
    emit dataChanged(index(0), index(rowCount()));
}

QLocale::Country LocaleGroupModel::country(QModelIndex index)
{
    return QLocale::countriesForLanguage(d->language).at(index.row());
}
