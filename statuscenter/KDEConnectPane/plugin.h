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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <QTranslator>
#include <statuscenterpane.h>
#include <QApplication>

class Plugin : public QObject, public StatusCenterPane
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID STATUS_CENTER_PANE_IID FILE "KdeConnectPane.json")
        Q_INTERFACES(StatusCenterPane)

    public:
        explicit Plugin(QObject *parent = 0);

        QList<StatusCenterPaneObject*> availablePanes();
        void loadLanguage();
    private:
        QList<StatusCenterPaneObject*> panes;

        QTranslator* translator;
};

#endif // PLUGIN_H
