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

#include "onboarding.h"
#include "ui_onboarding.h"
#include "internationalisation.h"

#define ChangelogOnbording \
    "New in theShell 8.0:\n"\
    "- Redshift can now be automatically timed according to the sunlight cycle based on your location\n"\
    "- Status Center panes now have a completely new and updated look\n"\
    "- The Compact Bar has been added\n"\
    "- Screen Recording is now built into theShell. Use SHIFT+PRTSCR or SUPER+ALT+O to start recording your screen.\n"\
    "- Switching users has now been made easier.\n"\
    "- Configuration of GTK3 themes have been added into the Theme section of System Settings.\n"\
    "- New Daylight Savings Time Changeover Notifications\n"\
    "- theShell will now be able to act as a geoclue agent and provide prompts when an app requests access to your location\n"\
    "- In the event of a crash, a new error screen with more information is displayed\n"\
    "- Startup and power off experience improved\n"\
    "- Right clicking on the bar is now possible\n"\
    "- Improved experience when switching between languages\n"\
    "- KDE Connect integration has been overhauled, with a new and more consistent interface\n"\
    "- Redesigned Desktop Background Selection dialog\n"\
    "- New Community Wallpapers with photos taken by the theShell community that are automatically cycled through for the desktop background\n"\
    "- Wallpapers now have sizing options\n"\
    "- Keyboard Layout is now configurable in theShell\n"\
    "- Updated design for authentication dialog when an app requiring authentication is required\n"\
    "- New Mouse Password! Use your mouse to unlock your computer so you don't have to type in your password every time\n"\
    "- New media player controls! Find them in the Notifications pane of the Status Center while a media player is open\n"\
    "- Notifications now show the time since the notification was posted\n"\
    "- Fixed a bug relating to DPI settings\n"\
    "- Fixed a bug relating to old autostarted apps starting up\n"\
    "\n"\
    "New in theShell 7.1:\n"\
    "- theWave has been discontinued.\n"\
    "- New Notifications system, allowing for simpler notifications to be shown\n"\
    "- Notifications can now be configured on an app by app basis. For example, you can block an app from sending notifications.\n"\
    "- Timezone can now be set in Settings\n"\
    "- Screen power saving settings can now be set in System Settings\n"\
    "- Quiet Mode can now be timed\n"\
    "- Desktop actions are now supported in the Gateway\n"\
    "- Flight Mode no longer displays the disconnected icon when you're disconnected from a network and in flight mode\n"\
    "- Power Stretch now persists between sessions\n"\
    "- Connecting to WPA Enterprise networks are now supported using theShell\n"\
    "- Autostarted apps now follow the XDG standard\n"\
    "\n"\
    "New in theShell 7.0:\n"\
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
extern float getDPIScaling();

Onboarding::Onboarding(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Onboarding)
{
    ui->setupUi(this);

    //ui->buttonBox->setVisible(false);
    ui->backButton->setVisible(false);
    ui->changelog->setText(ChangelogOnbording);
    ui->stackedWidget->setCurrentIndex(0, false);
    ui->welcomeLabel->setText(tr("Welcome to theShell %1!").arg(TS_VERSION));
    ui->tsLogo_2->setPixmap(QIcon::fromTheme("theshell").pixmap(256, 256));
    ui->iconLabel->setPixmap(QIcon(":/icons/icon.svg").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->exitStackedWidget->setCurrentAnimation(tStackedWidget::Lift);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

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
    ui->backButton->setVisible(true);
    ui->nextButton->setEnabled(true);
    ui->backButton->setVisible(true);
    ui->nextButton->setText(tr("Next"));
    switch (arg1) {
        case 0:
            ui->backButton->setVisible(false);
            ui->nextButton->setEnabled(true);
            break;
        case 1:
            ui->backButton->setVisible(false);
            break;
        case 3:
            ui->nextButton->setEnabled(false);
            break;
        case 4:
            ui->nextButton->setEnabled(false);
            break;
        case 5:
            ui->nextButton->setText(tr("Start"));
            break;
    }
}

void Onboarding::on_nextButton_clicked()
{
    if (ui->stackedWidget->currentIndex() == 5) {
        onboardingDone = true;
        this->accept();
    } else {
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
    }
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

void Onboarding::on_changeLanguageButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);

    //Fill language box
    Internationalisation::fillLanguageBox(ui->localeList);
}

void Onboarding::on_localeList_currentRowChanged(int currentRow)
{
    if (currentRow == -1) return;
    settings.setValue("locale/language", ui->localeList->item(currentRow)->data(Qt::UserRole).toString());

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

void Onboarding::on_exitStackedWidget_currentChanged(int arg1)
{
    if (arg1 == 0) {
        ui->nextButton->setVisible(true);
        ui->backButton->setVisible(true);
    } else {
        ui->nextButton->setVisible(false);
        ui->backButton->setVisible(false);
    }
}


void Onboarding::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        ui->welcomeLabel->setText(tr("Welcome to theShell %1!").arg(TS_VERSION));
    }
    QDialog::changeEvent(event);
}

void Onboarding::on_enableCompactBarButton_clicked()
{
    settings.setValue("bar/compact", true);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::on_disableCompactBarButton_clicked()
{
    settings.setValue("bar/compact", false);
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex() + 1);
}

void Onboarding::showFullScreen() {
    QPalette normalPalette = this->palette();

    QPalette blackPalette = normalPalette;
    blackPalette.setColor(QPalette::Window, Qt::black);
    this->setPalette(blackPalette);

    this->setWindowOpacity(0);

    QGraphicsOpacityEffect* buttonBoxOpacity = new QGraphicsOpacityEffect();
    buttonBoxOpacity->setOpacity(0);
    ui->buttonBox->setGraphicsEffect(buttonBoxOpacity);

    QGraphicsOpacityEffect* welcomeOpacity = new QGraphicsOpacityEffect();
    welcomeOpacity->setOpacity(0);
    ui->welcomeLabel->setGraphicsEffect(welcomeOpacity);

    QGraphicsOpacityEffect* optionsOpacity = new QGraphicsOpacityEffect();
    optionsOpacity->setOpacity(0);
    ui->optionsBox->setGraphicsEffect(optionsOpacity);

    QGraphicsOpacityEffect* iconOpacity = new QGraphicsOpacityEffect();
    iconOpacity->setOpacity(0);
    ui->tsLogo->setGraphicsEffect(iconOpacity);

    QDialog::showFullScreen();

    QTimer::singleShot(500, [=] {
        //Wait for window to load
        tPropertyAnimation* anim = new tPropertyAnimation(this, "windowOpacity");
        anim->setStartValue((float) 0);
        anim->setEndValue((float) 1);
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tPropertyAnimation::finished, [=] {
            anim->deleteLater();

            tVariantAnimation* iconAnim = new tVariantAnimation();
            iconAnim->setStartValue(QSize(200 * getDPIScaling(), 200 * getDPIScaling()));
            iconAnim->setEndValue(QSize(256 * getDPIScaling(), 256 * getDPIScaling()));
            iconAnim->setDuration(1000);
            iconAnim->setEasingCurve(QEasingCurve::OutCubic);
            connect(iconAnim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->tsLogo->setPixmap(QIcon::fromTheme("theshell").pixmap(value.toSize()));
            });
            QTimer::singleShot(800, [=] {
                tPropertyAnimation* buttonBoxAnim = new tPropertyAnimation(buttonBoxOpacity, "opacity");
                buttonBoxAnim->setStartValue((float) 0);
                buttonBoxAnim->setEndValue((float) 1);
                buttonBoxAnim->setDuration(500);
                buttonBoxAnim->setEasingCurve(QEasingCurve::OutCubic);
                buttonBoxAnim->start(tPropertyAnimation::DeleteWhenStopped);

                tPropertyAnimation* textAnim = new tPropertyAnimation(welcomeOpacity, "opacity");
                textAnim->setStartValue((float) 0);
                textAnim->setEndValue((float) 1);
                textAnim->setDuration(500);
                textAnim->setEasingCurve(QEasingCurve::OutCubic);
                textAnim->start(tPropertyAnimation::DeleteWhenStopped);

                tPropertyAnimation* optAnim = new tPropertyAnimation(optionsOpacity, "opacity");
                optAnim->setStartValue((float) 0);
                optAnim->setEndValue((float) 1);
                optAnim->setDuration(500);
                optAnim->setEasingCurve(QEasingCurve::OutCubic);
                optAnim->start(tPropertyAnimation::DeleteWhenStopped);

                tVariantAnimation* palAnim = new tVariantAnimation();
                palAnim->setStartValue(blackPalette.color(QPalette::Window));
                palAnim->setEndValue(normalPalette.color(QPalette::Window));
                palAnim->setDuration(500);
                palAnim->setEasingCurve(QEasingCurve::OutCubic);
                connect(palAnim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                    QPalette pal = this->palette();
                    pal.setColor(QPalette::Window, value.value<QColor>());
                    this->setPalette(pal);
                });
                palAnim->start(tPropertyAnimation::DeleteWhenStopped);
            });
            iconAnim->start(tVariantAnimation::DeleteWhenStopped);

            tPropertyAnimation* opacityAnim = new tPropertyAnimation(iconOpacity, "opacity");
            opacityAnim->setStartValue((float) 0);
            opacityAnim->setEndValue((float) 1);
            opacityAnim->setDuration(1000);
            opacityAnim->setEasingCurve(QEasingCurve::OutCubic);
            opacityAnim->start(tPropertyAnimation::DeleteWhenStopped);
        });
        anim->start();

    });
}
