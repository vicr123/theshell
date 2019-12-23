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
#ifndef LOCALEGROUPMODEL_H
#define LOCALEGROUPMODEL_H

#include <QLocale>
#include <QAbstractListModel>

struct LocaleGroupModelPrivate;
class LocaleGroupModel : public QAbstractListModel
{
        Q_OBJECT

    public:
        explicit LocaleGroupModel(QObject *parent = nullptr);
        ~LocaleGroupModel();

        // Basic functionality:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        void setLanguage(QLocale::Language language);
        QLocale::Country country(QModelIndex index);

    private:
        LocaleGroupModelPrivate* d;
};

#endif // LOCALEGROUPMODEL_H
