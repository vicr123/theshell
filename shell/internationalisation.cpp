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
