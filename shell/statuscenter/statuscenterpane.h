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

#ifndef STATUSCENTERPANE_H
#define STATUSCENTERPANE_H

#include <QList>
#include "statuscenterpaneobject.h"

class StatusCenterPane {
    public:
        virtual ~StatusCenterPane() {}

        virtual QList<StatusCenterPaneObject*> availablePanes() = 0;
        virtual void loadLanguage(QString language) = 0;
};

#define STATUS_CENTER_PANE_IID "org.thesuite.theshell.statuscenterpane"
Q_DECLARE_INTERFACE(StatusCenterPane, STATUS_CENTER_PANE_IID)

#endif // STATUSCENTERPANE_H
