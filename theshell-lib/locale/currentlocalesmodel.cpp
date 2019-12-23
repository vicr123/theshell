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
#include "currentlocalesmodel.h"

#include "localemanager.h"
#include <QIcon>

struct CurrentLocalesModelPrivate {
    QList<QLocale> currentLocales;
};

CurrentLocalesModel::CurrentLocalesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    d = new CurrentLocalesModelPrivate();
    d->currentLocales = LocaleManager::currentLocale();

    connect(LocaleManager::instance(), &LocaleManager::localeChanged, this, [=] {
        d->currentLocales = LocaleManager::currentLocale();
        emit dataChanged(index(0), index(rowCount()));
    });
}

CurrentLocalesModel::~CurrentLocalesModel()
{
    delete d;
}

int CurrentLocalesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return d->currentLocales.count() + 1;
}

QVariant CurrentLocalesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row() == d->currentLocales.count()) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("Add Language");
            case Qt::DecorationRole:
                return QIcon::fromTheme("list-add");
        }
    } else {
        QLocale loc = d->currentLocales.value(index.row());
        switch (role) {
            case Qt::DisplayRole: {
                QString text = QStringLiteral("%1 (%2)").arg(QLocale::languageToString(loc.language())).arg(loc.nativeLanguageName());
                if (index.row() == 0) {
                    text.append(" · " + tr("Primary"));
                }
                return text;
            }
        }
    }

    return QVariant();
}

QModelIndex CurrentLocalesModel::indexOf(QLocale locale)
{
    return index(d->currentLocales.indexOf(locale));
}

QLocale CurrentLocalesModel::locale(QModelIndex index)
{
    return d->currentLocales.at(index.row());
}
