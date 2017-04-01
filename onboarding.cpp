#include "onboarding.h"
#include "ui_onboarding.h"

#define ChangelogOnbording "New in theShell 6.1:\n"\
    "- New Status Bar. Go to Settings > Bar to activate it.\n"\
    "- Preliminary touch support\n"\
    "- Region selection in screenshots\n"\
    "- New Danish, Dutch and Portuguese translations\n"\
    "- Translations are hotswappable\n"\
    "\n"\
    "New in theShell 6.0:\n"\
    "- Default system font can be set in theme settings.\n"\
    "- Battery indicator now shows battery for devices connected over KDE Connect\n"\
    "- Added some KDE Connect actions in the status center\n"\
    "- The stopwatch is now shown on the bar when being used.\n"\
    "- New Charge History in the Status Center. Go to the System Status pane in the Status Center to take a look.\n"\
    "- Add scrollbar on the bar when content runs off the bar\n"\
    "- Added Power Stretch.\n"\
    "- Added icon in bar for wireless reception\n"\
    "- theShell now uses the-libs for integration with other the-applications.\n"\
    "- Dragging down on an item in the bar to open the status center has been added\n"\
    "- New notification animation\n"\
    "- When a timer elapses, other audio is made quiter until the timer is dismissed\n"\
    "- Redshift now works overnight\n"\
    "- Notifications can now send sounds through theShell\n"\
    "- theWave now quitens background music while listening\n"\
    "- Time and date can now be set within theShell\n"\
    "- System Users can now be configured within theShell\n"\
    "- theShell now supports localisations! Try the inbuilt Vietnamese translation by changing the settings in theShell\n"\
    "- theShell's settings now apply to the whole system, so there is no need for a seperate system settings application. More settings panes will be coming soon.\n"\
    "\n"\
    "New in theShell 5.2: \n"\
    "- New Onboarding experience. Every time theShell updates, you'll get a changelog.\n" \
    "- theWave now uses theCalculator to process calculations.\n" \
    "- theShell now uses its own platform abstraction module to theme your system. Change the theming in theShell Settings.\n" \
    "- theShell now uses the Contemporary theme to display widgets.\n"\
    "- theShell now displays notifications when it detects a connected device\n" \
    ""

extern NativeEventFilter* NativeFilter;
extern QTranslator *qtTranslator, *tsTranslator;

enum languageOrder {
    enUS = 0,
    enGB,
    enAU,
    enNZ,
    viVN,
    daDK,
    ptBR,
    jaJP,
    arSA,
    zhCN,
    nlNL,
    deDE,
    miNZ,
    maxLanguage
};


Onboarding::Onboarding(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Onboarding)
{
    ui->setupUi(this);

    ui->buttonBox->setVisible(false);
    ui->changelog->setText(ChangelogOnbording);
    ui->thewaveLogo->setPixmap(QIcon(":/icons/thewave.svg").pixmap(256, 256));
    ui->stackedWidget->setCurrentIndex(0);

    QTimer* timer = new QTimer(this);
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, [=] {
        buttonCurrentLanguage++;

        if (buttonCurrentLanguage == 7) buttonCurrentLanguage = 0;

        switch (buttonCurrentLanguage) {
            case 0:
                ui->changeLanguageButton->setText("Language");
                break;
            case 1:
                ui->changeLanguageButton->setText("Taal");
                break;
            case 2:
                ui->changeLanguageButton->setText("Ngôn ngữ");
                break;
            case 3:
                ui->changeLanguageButton->setText("Linguagem");
                break;
            case 4:
                ui->changeLanguageButton->setText("Sprog");
                break;
            case 5:
                ui->changeLanguageButton->setText("Sprache");
                break;
            case 6:
                ui->changeLanguageButton->setText("Langue");
                break;
        }
    });
    timer->start();

    connect(NativeFilter, &NativeEventFilter::DoRetranslation, [=] {
        ui->retranslateUi(this);
    });
}

Onboarding::~Onboarding()
{
    delete ui;
}

void Onboarding::on_closeButton_clicked()
{
    this->close();
}

void Onboarding::on_stackedWidget_currentChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
    ui->nextButton->setVisible(true);
    ui->backButton->setVisible(true);
    switch (arg1) {
        case 0:
            ui->backButton->setVisible(false);
            break;
        case 1:
            ui->buttonBox->setVisible(false);
            break;
        case 4:
            ui->buttonBox->setVisible(false);
            break;
        case 3:
            ui->nextButton->setVisible(false);
            break;
    }
}

void Onboarding::on_nextButtonFirstPage_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_nextButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() - 1);
}

void Onboarding::on_beginButton_clicked()
{
    this->close();
}

void Onboarding::on_enabletheWaveButton_clicked()
{
    settings.setValue("thewave/enabled", true);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_disabletheWaveButton_clicked()
{
    settings.setValue("thewave/enabled", false);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_changeLanguageButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    ui->localeList->clear();
    //For the time being, we'll just have hardcoded locales. This should change soon (hopefully)
    for (int i = 0; i < maxLanguage; i++) {
        switch (i) {
            case enUS:
                ui->localeList->addItem("[US] " + tr("English") + " (English)");
                break;
            case enGB:
                ui->localeList->addItem("[GB] " + tr("English") + " (English)");
                break;
            case enAU:
                ui->localeList->addItem("[AU] " + tr("English") + " (English)");
                break;
            case enNZ:
                ui->localeList->addItem("[NZ] " + tr("English") + " (English)");
                break;
            case viVN:
                ui->localeList->addItem("[VN] " + tr("Vietnamese") + " (Tiếng Việt)");
                break;
            case daDK:
                ui->localeList->addItem("[DK] " + tr("Danish") + " (Dansk) ");
                break;
            case ptBR:
                ui->localeList->addItem("[BR] " + tr("Portuguese") + " (Português) ");
                break;
            case arSA:
                ui->localeList->addItem("[SA] " + tr("Arabic") + " (العربية) ");
                break;
            case zhCN:
                ui->localeList->addItem("[CN] " + tr("Chinese") + " (中文) ");
                break;
            case nlNL:
                ui->localeList->addItem("[NL] " + tr("Dutch") + " (Nederlands) ");
                break;
            case miNZ:
                ui->localeList->addItem("[NZ] " + tr("Māori") + " (Māori) ");
                break;
            case jaJP:
                ui->localeList->addItem("[JP] " + tr("Japanese") + " (日本語) ");
                break;
            case deDE:
                ui->localeList->addItem("[DE] " + tr("German") + " (Deutsch) ");
                break;
        }
    }

    QString currentLocale = settings.value("locale/language", "en_US").toString();
    if (currentLocale == "en_US") {
        ui->localeList->setCurrentRow(enUS);
    } else if (currentLocale == "en_GB") {
        ui->localeList->setCurrentRow(enGB);
    } else if (currentLocale == "en_AU") {
        ui->localeList->setCurrentRow(enAU);
    } else if (currentLocale == "en_NZ") {
        ui->localeList->setCurrentRow(enNZ);
    } else if (currentLocale == "vi_VN") {
        ui->localeList->setCurrentRow(viVN);
    } else if (currentLocale == "da_DK") {
        ui->localeList->setCurrentRow(daDK);
    } else if (currentLocale == "pt_BR") {
        ui->localeList->setCurrentRow(ptBR);
    } else if (currentLocale == "ar_SA") {
        ui->localeList->setCurrentRow(arSA);
    } else if (currentLocale == "zh_CN") {
        ui->localeList->setCurrentRow(zhCN);
    } else if (currentLocale == "nl_NL") {
        ui->localeList->setCurrentRow(nlNL);
    } else if (currentLocale == "mi_NZ") {
        ui->localeList->setCurrentRow(miNZ);
    } else if (currentLocale == "ja_JP") {
        ui->localeList->setCurrentRow(jaJP);
    } else if (currentLocale == "de_DE") {
        ui->localeList->setCurrentRow(deDE);
    }
}

void Onboarding::on_localeList_currentRowChanged(int currentRow)
{
    switch (currentRow) {
        case enUS:
            settings.setValue("locale/language", "en_US");
            break;
        case enGB:
            settings.setValue("locale/language", "en_GB");
            break;
        case enAU:
            settings.setValue("locale/language", "en_AU");
            break;
        case enNZ:
            settings.setValue("locale/language", "en_NZ");
            break;
        case viVN:
            settings.setValue("locale/language", "vi_VN");
            break;
        case daDK:
            settings.setValue("locale/language", "da_DK");
            break;
        case ptBR:
            settings.setValue("locale/language", "pt_BR");
            break;
        case arSA:
            settings.setValue("locale/language", "ar_SA");
            break;
        case zhCN:
            settings.setValue("locale/language", "zh_CN");
            break;
        case nlNL:
            settings.setValue("locale/language", "nl_NL");
            break;
        case miNZ:
            settings.setValue("locale/language", "mi_NZ");
            break;
        case jaJP:
            settings.setValue("locale/language", "ja_JP");
            break;
        case deDE:
            settings.setValue("locale/language", "de_DE");
            break;
    }

    QString localeName = settings.value("locale/language", "en_US").toString();
    qputenv("LANG", localeName.toUtf8());

    QLocale defaultLocale(localeName);
    QLocale::setDefault(defaultLocale);

    if (defaultLocale.language() == QLocale::Arabic || defaultLocale.language() == QLocale::Hebrew) {
        //Reverse the layout direction
        QApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        //Set normal layout direction
        QApplication::setLayoutDirection(Qt::LeftToRight);
    }

    qtTranslator->load("qt_" + defaultLocale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QApplication::installTranslator(qtTranslator);

    qDebug() << QLocale().name();

    tsTranslator->load(QLocale().name(), "/usr/share/theshell/translations");
    QApplication::installTranslator(tsTranslator);

    emit NativeFilter->DoRetranslation();
}
