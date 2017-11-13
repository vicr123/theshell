/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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
        viVN,
        daDK,
        esES,
        ruRU,
        svSE,
        ltLT,
        plPL,
        // Unfinished languages follow
        jaJP,
        arSA,
        zhCN,
        deDE,
        frFR,
        miNZ,
        ptBR,
        maxLanguage
    };

    static void fillLanguageBox(QListWidget* languageBox) {
        //Clear the box
        languageBox->clear();

        //Block signals
        languageBox->blockSignals(true);

        //For the time being, we'll just have hardcoded locales. This should change soon (hopefully)
        for (int i = 0; i < maxLanguage; i++) {
            switch (i) {
                case enUS:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-us"), tr("English") + " (English)"));
                    break;
                case enGB:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-uk"), tr("English") + " (English)"));
                    break;
                case enAU:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-au"), tr("English") + " (English)"));
                    break;
                case enNZ:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-nz"), tr("English") + " (English)"));
                    break;
                case viVN:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-vn"), tr("Vietnamese") + " (Tiếng Việt)"));
                    break;
                case daDK:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-dk"), tr("Danish") + " (Dansk)"));
                    break;
                case nlNL:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-nl"), tr("Dutch") + " (Nederlands)"));
                    break;
                case esES:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-es"), tr("Spanish") + " (Español)"));
                    break;
                case ruRU:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-ru"), tr("Russian") + " (русский)"));
                    break;
                case svSE:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-se"), tr("Swedish") + " (Svenska)"));
                    break;
                case ltLT:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-lt"), tr("Lithuanian") + " (Lietuviškai)"));
                    break;
                case plPL:
                    languageBox->addItem(new QListWidgetItem(QIcon::fromTheme("flag-pl"), tr("Polish") + " (Polski)"));
                    break;
                case arSA:
                    //languageBox->addItem("[SA] " + tr("Arabic") + " (العربية) ");
                    break;
                case ptBR:
                    //languageBox->addItem("[BR] " + tr("Portuguese") + " (Português) ");
                    break;
                case zhCN:
                    //languageBox->addItem("[CN] " + tr("Chinese") + " (中文) ");
                    break;
                case frFR:
                    //languageBox->addItem("[FR] " + tr("French") + " (Français) ");
                    break;
                case miNZ:
                    //languageBox->addItem("[NZ] " + tr("Māori") + " (Māori) ");
                    break;
                case jaJP:
                    //languageBox->addItem("[JP] " + tr("Japanese") + " (日本語) ");
                    break;
                case deDE:
                    //languageBox->addItem("[DE] " + tr("German") + " (Deutsch) ");
                    break;
            }
        }

        if (false) {
            Q_UNUSED(QT_TR_NOOP("Afrikaans"));
            Q_UNUSED(QT_TR_NOOP("Arabic"));
            Q_UNUSED(QT_TR_NOOP("Bulgarian"));
            Q_UNUSED(QT_TR_NOOP("Catalan"));
            Q_UNUSED(QT_TR_NOOP("Chinese"));
            Q_UNUSED(QT_TR_NOOP("Croatian"));
            Q_UNUSED(QT_TR_NOOP("Czech"));
            Q_UNUSED(QT_TR_NOOP("Estonian"));
            Q_UNUSED(QT_TR_NOOP("Finnish"));
            Q_UNUSED(QT_TR_NOOP("French"));
            Q_UNUSED(QT_TR_NOOP("German"));
            Q_UNUSED(QT_TR_NOOP("Greek"));
            Q_UNUSED(QT_TR_NOOP("Hebrew"));
            Q_UNUSED(QT_TR_NOOP("Hungarian"));
            Q_UNUSED(QT_TR_NOOP("Italian"));
            Q_UNUSED(QT_TR_NOOP("Icelandic"));
            Q_UNUSED(QT_TR_NOOP("Indonesian"));
            Q_UNUSED(QT_TR_NOOP("Japanese"));
            Q_UNUSED(QT_TR_NOOP("Korean"));
            Q_UNUSED(QT_TR_NOOP("Latvian"));
            Q_UNUSED(QT_TR_NOOP("Māori"));
            Q_UNUSED(QT_TR_NOOP("Norwegian"));
            Q_UNUSED(QT_TR_NOOP("Polish"));
            Q_UNUSED(QT_TR_NOOP("Portuguese"));
            Q_UNUSED(QT_TR_NOOP("Serbian"));
            Q_UNUSED(QT_TR_NOOP("Slovak"));
            Q_UNUSED(QT_TR_NOOP("Slovenian"));
            Q_UNUSED(QT_TR_NOOP("Tagalog"));
            Q_UNUSED(QT_TR_NOOP("Thai"));
            Q_UNUSED(QT_TR_NOOP("Turkish"));
            Q_UNUSED(QT_TR_NOOP("Ukranian"));
        }

        QSettings settings;

        QString currentLocale = settings.value("locale/language", "en_US").toString();
        if (currentLocale == "en_US") {
            languageBox->setCurrentRow(enUS);
        } else if (currentLocale == "en_GB") {
            languageBox->setCurrentRow(enGB);
        } else if (currentLocale == "en_AU") {
            languageBox->setCurrentRow(enAU);
        } else if (currentLocale == "en_NZ") {
            languageBox->setCurrentRow(enNZ);
        } else if (currentLocale == "vi_VN") {
            languageBox->setCurrentRow(viVN);
        } else if (currentLocale == "da_DK") {
            languageBox->setCurrentRow(daDK);
        } else if (currentLocale == "pt_BR") {
            languageBox->setCurrentRow(ptBR);
        } else if (currentLocale == "ar_SA") {
            languageBox->setCurrentRow(arSA);
        } else if (currentLocale == "zh_CN") {
            languageBox->setCurrentRow(zhCN);
        } else if (currentLocale == "nl_NL") {
            languageBox->setCurrentRow(nlNL);
        } else if (currentLocale == "mi_NZ") {
            languageBox->setCurrentRow(miNZ);
        } else if (currentLocale == "ja_JP") {
            languageBox->setCurrentRow(jaJP);
        } else if (currentLocale == "de_DE") {
            languageBox->setCurrentRow(deDE);
        } else if (currentLocale == "fr_FR") {
            languageBox->setCurrentRow(frFR);
        } else if (currentLocale == "es_ES") {
            languageBox->setCurrentRow(esES);
        } else if (currentLocale == "ru_RU") {
            languageBox->setCurrentRow(ruRU);
        } else if (currentLocale == "sv_SE") {
            languageBox->setCurrentRow(svSE);
        } else if (currentLocale == "lt_LT") {
            languageBox->setCurrentRow(ltLT);
        }

        //Unblock signals
        languageBox->blockSignals(false);
    }
};

#endif // INTERNATIONALISATION_H
