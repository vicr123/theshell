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

#include "onboarding.h"
#include "ui_onboarding.h"
#include "internationalisation.h"

#define ChangelogOnbording "New in theShell 7.0:\n"\
    "- Quiet Mode has been added to theShell! Click on the volume icon in the bar, or head to the Notifications Status Center pane to change the setting.\n"\
    "- The Gateway has been revamped and now shows your apps properly, as well as updates automatically whenever you install a new app.\n"\
    "- Flight Mode has been added\n"\
    "- The Bluetooth Switch now works properly\n"\
    "- Location icon now appears in the bar when your location is actively being used (for example, in GNOME Maps)\n"\
    "- The system icon theme can now be changed in System Settings\n"\
    "- Text on the bar has been shortened and made more concise\n"\
    "- The clock now supports 12 hour mode. Change it in System Settings\n"\
    "- Rate History and Application Power Usage is now included in the System Status Status Center pane\n"\
    "- The About Settings pane now shows system information\n"\
    "- The bar can now be moved to the bottom of the screen\n"\
    "- New Automatically Show Bar option. The bar will not extend unless it is clicked on if this option is not set.\n"\
    "- New network manager\n"\
    "- Printers removed\n"\
    "- theShell Session Manager is now used to start theShell. If theShell crashes, you'll be able to restart it.\n"\
    "- Added DPI settings. On high DPI monitors, theShell can be scaled up.\n"\
    "- Power Stretch now disables the animation in the Status Center\n"\
    "- Added some accessibility settings.\n"\
    "- Added option to keep screen unlocked after suspend\n"\
    "- Changing any theme settings will now automatically change them in all supported applications.\n"\
    "- New end session screen when pressing power button or CTRL+ALT+DEL\n"\
    "- User Management now sets passwords correctly\n"\
    "- Administrator and standard accounts can now be set in user management\n"\
    "- New lock screen\n"\
    "\n"\
    "New in theShell 6.1:\n"\
    "- New Status Bar. Go to Settings > Bar to activate it.\n"\
    "- Preliminary touch support\n"\
    "- Region selection in screenshots\n"\
    "- New translations\n"\
    "- Translations are hotswappable\n"\
    "- New key combination! To bring down the status center, use SUPER+F1-F6!\n"\
    "- Gateway opens faster\n"\
    "- Contemporary icon set has been released and is the default for theShell\n"\
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
    "- theShell now displays notifications when it detects a connected device\n"\
    ""

extern NativeEventFilter* NativeFilter;
extern QTranslator *qtTranslator, *tsTranslator;

Onboarding::Onboarding(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Onboarding)
{
    ui->setupUi(this);

    ui->buttonBox->setVisible(false);
    ui->changelog->setText(ChangelogOnbording);
    ui->thewaveLogo->setPixmap(QIcon(":/icons/thewave.svg").pixmap(256, 256));
    ui->stackedWidget->setCurrentIndex(0);
    ui->welcomeLabel->setText(tr("Welcome to theShell %1!").arg(TS_VERSION));

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
        ui->welcomeLabel->setText(tr("Welcome to theShell %1!").arg(TS_VERSION));
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
        case 3:
        case 4:
            ui->nextButton->setVisible(false);
            break;
        case 5:
            ui->buttonBox->setVisible(false);
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
    onboardingDone = true;
    this->accept();
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

    //Fill language box
    Internationalisation::fillLanguageBox(ui->localeList);
}

void Onboarding::on_localeList_currentRowChanged(int currentRow)
{
    switch (currentRow) {
        case Internationalisation::enUS:
            settings.setValue("locale/language", "en_US");
            break;
        case Internationalisation::enGB:
            settings.setValue("locale/language", "en_GB");
            break;
        case Internationalisation::enAU:
            settings.setValue("locale/language", "en_AU");
            break;
        case Internationalisation::enNZ:
            settings.setValue("locale/language", "en_NZ");
            break;
        case Internationalisation::viVN:
            settings.setValue("locale/language", "vi_VN");
            break;
        case Internationalisation::daDK:
            settings.setValue("locale/language", "da_DK");
            break;
        case Internationalisation::ptBR:
            settings.setValue("locale/language", "pt_BR");
            break;
        case Internationalisation::arSA:
            settings.setValue("locale/language", "ar_SA");
            break;
        case Internationalisation::zhCN:
            settings.setValue("locale/language", "zh_CN");
            break;
        case Internationalisation::nlNL:
            settings.setValue("locale/language", "nl_NL");
            break;
        case Internationalisation::miNZ:
            settings.setValue("locale/language", "mi_NZ");
            break;
        case Internationalisation::jaJP:
            settings.setValue("locale/language", "ja_JP");
            break;
        case Internationalisation::deDE:
            settings.setValue("locale/language", "de_DE");
            break;
        case Internationalisation::esES:
            settings.setValue("locale/language", "es_ES");
            break;
        case Internationalisation::ruRU:
            settings.setValue("locale/language", "ru_RU");
            break;
        case Internationalisation::svSE:
            settings.setValue("locale/language", "sv_SE");
            break;
        case Internationalisation::ltLT:
            settings.setValue("locale/language", "lt_LT");
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

    tsTranslator->load(QLocale().name(), QString(SHAREDIR) + "translations");
    QApplication::installTranslator(tsTranslator);

    //Fill locale box
    Internationalisation::fillLanguageBox(ui->localeList);

    emit NativeFilter->DoRetranslation();
}

void Onboarding::on_enableStatusBarButton_clicked()
{
    settings.setValue("bar/statusBar", true);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);

}

void Onboarding::on_disableStatusBarButton_clicked()
{
    settings.setValue("bar/statusBar", false);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::reject() {
    if (onboardingDone) {
        QDialog::reject();
    } else {
        ui->exitStackedWidget->setCurrentIndex(1);
    }
}

void Onboarding::on_backToSetupButton_clicked()
{
    ui->exitStackedWidget->setCurrentIndex(0);
}

void Onboarding::on_logoutButton_clicked()
{
    QDialog::reject();
}

void Onboarding::on_powerOffButton_clicked()
{
    //Power off the PC
    QDBusMessage message;
    QList<QVariant> arguments;
    arguments.append(true);
    message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}
