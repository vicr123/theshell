#ifndef INTERNATIONALISATION_H
#define INTERNATIONALISATION_H

#include <QListWidget>
#include <QApplication>
#include <QSettings>

class Internationalisation {
private:
    static QString tr(const char* text) {
        return QApplication::translate("InfoPaneDropdown", text);
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
        ptBR,
        // Unfinished languages follow
        jaJP,
        arSA,
        zhCN,
        deDE,
        frFR,
        miNZ,
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
                    languageBox->addItem("[US] " + tr("English") + " (English)");
                    break;
                case enGB:
                    languageBox->addItem("[GB] " + tr("English") + " (English)");
                    break;
                case enAU:
                    languageBox->addItem("[AU] " + tr("English") + " (English)");
                    break;
                case enNZ:
                    languageBox->addItem("[NZ] " + tr("English") + " (English)");
                    break;
                case viVN:
                    languageBox->addItem("[VN] " + tr("Vietnamese") + " (Tiếng Việt)");
                    break;
                case daDK:
                    languageBox->addItem("[DK] " + tr("Danish") + " (Dansk) ");
                    break;
                case ptBR:
                    languageBox->addItem("[BR] " + tr("Portuguese") + " (Português) ");
                    break;
                case nlNL:
                    languageBox->addItem("[NL] " + tr("Dutch") + " (Nederlands) ");
                    break;
                case esES:
                    languageBox->addItem("[ES] " + tr("Spanish") + " (Español) ");
                    break;
                /*case arSA:
                    languageBox->addItem("[SA] " + tr("Arabic") + " (العربية) ");
                    break;
                case zhCN:
                    languageBox->addItem("[CN] " + tr("Chinese") + " (中文) ");
                    break;
                case frFR:
                    languageBox->addItem("[FR] " + tr("French") + " (Français) ");
                    break;
                case miNZ:
                    languageBox->addItem("[NZ] " + tr("Māori") + " (Māori) ");
                    break;
                case jaJP:
                    languageBox->addItem("[JP] " + tr("Japanese") + " (日本語) ");
                    break;
                case deDE:
                    languageBox->addItem("[DE] " + tr("German") + " (Deutsch) ");
                    break;*/
            }
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
        }

        //Unblock signals
        languageBox->blockSignals(false);
    }
};

#endif // INTERNATIONALISATION_H
