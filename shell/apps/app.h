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

#ifndef APP_H
#define APP_H

#include <QObject>
#include <QIcon>
#include <QMap>
#include <QVariant>
#include "app.h"

class App
{

public:
    explicit App();

    QString name() const;
    void setName(QString name);

    QIcon icon() const;
    void setIcon(QIcon icon);

    QString command() const;
    void setCommand(QString command);

    QString description() const;
    void setDescription(QString desc);

    bool isPinned() const;
    void setPinned(bool pinned);

    QString desktopEntry() const;
    void setDesktopEntry(QString entry);

    QList<App> actions() const;
    void setActions(QList<App> actions);
    void addAction(App action);

    QVariant additionalProperty(QString key);
    bool hasAdditionalProperty(QString key);
    void setAdditionalProperty(QString key, QVariant property);


    bool invalid();

    static App invalidApp();

    bool operator <(const App& other) const {
        int compare = name().localeAwareCompare(other.name());
        if (compare < 0) {
            return true;
        } else {
            return false;
        }
    }

    bool operator >(const App& other) const {
        return other < *this;
    }
signals:

public slots:

private:
    QString appname;
    QIcon appicon;
    QString appcommand;
    QString appdesc = "";
    QString appfile = "";
    bool pin = false;
    bool isInvalid = false;
    QList<App> acts;
    QMap<QString, QVariant> additional;
};


Q_DECLARE_METATYPE(App)

#endif // APP_H
