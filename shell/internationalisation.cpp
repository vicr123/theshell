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

#include "internationalisation.h"

#include <QDirIterator>

void Internationalisation::fillLanguageBox(QListWidget* languageBox) {
    //Clear the box
    languageBox->clear();

    //Block signals
    languageBox->blockSignals(true);

    QSettings settings;
    QString currentLocale = settings.value("locale/language", "en_US").toString();

    //Iterate over all available translations
    QDirIterator iterator(QString(SHAREDIR) + "translations", QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
       iterator.next();
       QFileInfo file = iterator.fileInfo();
       if (file.suffix() == "qm") { //Found a translation file
           QString localeName = file.baseName();
           QString country;
           if (localeName.contains("_")) {
               country = localeName.mid(localeName.indexOf("_") + 1);
           }
           QLocale locale(localeName);

           QString languageName = locale.languageToString(locale.language());
           QString nativeLanguageName = locale.nativeLanguageName();

           if (localeName == "au_AU") {
               languageName = "Australian";
               nativeLanguageName = "Aussie";
           }

           QListWidgetItem* item = new QListWidgetItem;
           item->setIcon(QIcon::fromTheme("flag-" + (country == "GB" ? "uk" : country.toLower()), QIcon::fromTheme("flag")));
           item->setText(QString("%1 (%2)").arg(languageName, nativeLanguageName));
           item->setData(Qt::UserRole, localeName);
           languageBox->addItem(item);

           if (localeName == currentLocale) {
               item->setSelected(true);
           }
       }
    }

    languageBox->sortItems();

    //Unblock signals
    languageBox->blockSignals(false);
}
