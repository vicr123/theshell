/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#ifndef INTERNATIONALISATION_H
#define INTERNATIONALISATION_H

#include <QListWidget>
#include <QApplication>
#include <QSettings>

class Internationalisation {
private:
    static QString tr(const char* text) {
        QString retval;
        retval = QApplication::translate("Internationalisation", text);
        if (strcmp(retval.toUtf8().data(), text) == 0) {
            retval = QApplication::translate("InfoPaneDropdown", text);
        }
        return retval;
    }

public:
    enum languageOrder {
        enUS = 0,
        enGB,
        enAU,
        enNZ,
        nlNL,
        deDE,
        viVN,
        daDK,
        esES,
        ruRU,
        svSE,
        ltLT,
        auAU,
        // Unfinished languages follow
        plPL,
        jaJP,
        arSA,
        zhCN,
        frFR,
        miNZ,
        ptBR,
        idID,
        nbNO,
        maxLanguage
    };

    static void fillLanguageBox(QListWidget* languageBox);
};

#endif // INTERNATIONALISATION_H
