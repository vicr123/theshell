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

#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"
#include "internationalisation.h"

#include <QScroller>
#include <tvirtualkeyboard.h>
#include "location/locationdaemon.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void EndSession(EndSessionWait::shutdownType type);
extern QString calculateSize(quint64 size);
extern AudioManager* AudioMan;
extern NativeEventFilter* NativeFilter;
extern QTranslator *qtTranslator, *tsTranslator;
extern float getDPIScaling();
extern QDBusServiceWatcher* dbusServiceWatcher;
extern QDBusServiceWatcher* dbusServiceWatcherSystem;
extern UPowerDBus* updbus;
extern NotificationsDBusAdaptor* ndbus;
extern DBusSignals* dbusSignals;
extern LocationServices* locationServices;
extern LocationDaemon* geolocation;


#define LOWER_INFOPANE InfoPaneNotOnTopLocker locker(this);

InfoPaneDropdown::InfoPaneDropdown(WId MainWindowId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    if (false) {
        Q_UNUSED(QT_TR_NOOP("Location"));
    }

    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    ui->settingsList->setIconSize(QSize(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->settingsListContainer->setFixedWidth(250 * getDPIScaling());

    startTime.start();

    ui->copyrightNotice->setText(tr("Copyright © Victor Tran %1. Licensed under the terms of the GNU General Public License, version 3 or later.").arg("2018"));
    ui->usesLocation->setPixmap(QIcon::fromTheme("gps").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));

    connect(this, SIGNAL(flightModeChanged(bool)), ui->NetworkManager, SLOT(flightModeChanged(bool)));
    connect(this, SIGNAL(flightModeChanged(bool)), ui->networkManagerSettings, SLOT(flightModeChanged(bool)));

    if (settings.value("flightmode/on", false).toBool()) {
        ui->FlightSwitch->setChecked(true);
    }

    this->MainWindowId = MainWindowId;

    connect(ui->notificationsWidget, SIGNAL(numNotificationsChanged(int)), this, SIGNAL(numNotificationsChanged(int)));

    connect(dbusServiceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(DBusServiceRegistered(QString)));
    connect(dbusServiceWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(DBusServiceUnregistered(QString)));
    connect(dbusServiceWatcherSystem, SIGNAL(serviceRegistered(QString)), this, SLOT(DBusServiceRegistered(QString)));
    connect(dbusServiceWatcherSystem, SIGNAL(serviceUnregistered(QString)), this, SLOT(DBusServiceUnregistered(QString)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateSysInfo()));
    timer->setInterval(1000);
    timer->start();

    connect(updbus, &UPowerDBus::powerStretchChanged, [=](bool isOn) {
        ui->PowerStretchSwitch->setChecked(isOn);
        emit batteryStretchChanged(isOn);
        doNetworkCheck();

        if (isOn) {
            slice1.pause();
            slice2.pause();
        } else {
            slice1.resume();
            slice2.resume();
        }
    });
    ui->PowerStretchSwitch->setChecked(updbus->powerStretch());
    emit batteryStretchChanged(updbus->powerStretch());

    ui->BatteryChargeScrollBar->setVisible(false);
    ui->appNotificationsConfigureLock->setVisible(false);
    ui->quietModeExtras->setFixedHeight(0);
    //ui->networkKey->setVisible(false);
    //ui->networkConnect->setVisible(false);
    ui->resetButton->setProperty("type", "destructive");
    ui->userSettingsDeleteUser->setProperty("type", "destructive");
    ui->userSettingsDeleteUserAndData->setProperty("type", "destructive");
    ui->userSettingsDeleteUserOnly->setProperty("type", "destructive");
    ui->resetDeviceButton->setProperty("type", "destructive");
    ui->partFrame->installEventFilter(this);

    QPalette powerStretchPalette = ui->PowerStretchSwitch->palette();
    QPalette flightModePalette = ui->FlightSwitch->palette();

    QColor background = this->palette().color(QPalette::Background);
    int averageCol = (background.red() + background.green() + background.blue()) / 3;

    if (averageCol < 127) {
        powerStretchPalette.setColor(QPalette::Highlight, QColor(255, 100, 0));
        powerStretchPalette.setColor(QPalette::WindowText, Qt::white);
        flightModePalette.setColor(QPalette::Highlight, QColor(0, 0, 200));
        flightModePalette.setColor(QPalette::WindowText, Qt::white);
    } else {
        powerStretchPalette.setColor(QPalette::Highlight, QColor(255, 200, 0));
        powerStretchPalette.setColor(QPalette::WindowText, Qt::black);
        flightModePalette.setColor(QPalette::Highlight, QColor(100, 200, 255));
        flightModePalette.setColor(QPalette::WindowText, Qt::black);
    }
    ui->PowerStretchSwitch->setPalette(powerStretchPalette);
    ui->FlightSwitch->setPalette(flightModePalette);

    //Set up battery chart
    batteryChart = new QChart();
    batteryChart->setBackgroundVisible(false);
    batteryChart->legend()->hide();

    QChartView* batteryChartView = new QChartView(batteryChart);
    batteryChartView->setRenderHint(QPainter::Antialiasing);
    ((QBoxLayout*) ui->batteryGraph->layout())->insertWidget(1, batteryChartView);

    updateBatteryChart();

    //Check Redshift
    QProcess* redshiftQuery = new QProcess;
    redshiftQuery->start("redshift -V");
    connect(redshiftQuery, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        QString output = redshiftQuery->readAll().trimmed();
        output.remove("redshift ");

        QStringList parts = output.split(".");
        for (int i = 0; i < parts.count(); i++) {
            int version = parts.at(i).toInt();
            if (i == 0) {
                if (version > 1) {
                    break;
                } else if (version < 1) {
                    isNewRedshift = false;
                    break;
                }
            } else if (i == 1) {
                if (version < 11) {
                    isNewRedshift = false;
                }
                break;
            }
        }
        redshiftQuery->deleteLater();
    });

    if (!QFile("/usr/bin/scallop").exists()) {
        ui->resetDeviceButton->setVisible(false);
    }

    //Set up networking
    QDBusInterface networkInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
    connect(ui->NetworkManager, SIGNAL(updateBarDisplay(QString,QIcon)), this, SIGNAL(networkLabelChanged(QString,QIcon)));

    ui->WifiSwitch->setChecked(networkInterface.property("WirelessEnabled").toBool());

    {
        QDBusInterface interface("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", QDBusConnection::sessionBus());
        QDBusConnection::sessionBus().connect("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", "BluetoothEnabledChanged", this, SLOT(bluetoothEnabledChanged()));

        if (interface.isValid()) {
            DBusServiceRegistered("org.thesuite.tsbt");
        } else {
            DBusServiceUnregistered("org.thesuite.tsbt");
        }

        dbusServiceWatcher->addWatchedService("org.thesuite.tsbt");
    }

    //Load icons into icon theme box
    {
        QString currentIconTheme = themeSettings->value("icons/theme", "contemporary").toString();
        QDir iconPath("/usr/share/icons");
        for (QString iconDir : iconPath.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
            QFile themeFile("/usr/share/icons/" + iconDir + "/index.theme");
            if (themeFile.exists()) {
                themeFile.open(QFile::ReadOnly);
                QString iconThemeName = "";

                while (!themeFile.atEnd()) {
                    QString line = themeFile.readLine();
                    if (line.startsWith("Name")) {
                        iconThemeName = line.mid(line.indexOf("=") + 1).remove("\n");
                        break;
                    }
                }

                themeFile.close();

                if (iconThemeName != "") {
                    ui->systemIconTheme->addItem(iconThemeName);
                    ui->systemIconTheme->setItemData(ui->systemIconTheme->count() - 1, iconDir);

                    if (currentIconTheme == iconDir) {
                        ui->systemIconTheme->setCurrentIndex(ui->systemIconTheme->count() - 1);
                    }
                }
            }
        }
    }

    //Load widget themes into widget theme box
    {
        ui->systemWidgetTheme->blockSignals(true);
        QString currentWidgetTheme = themeSettings->value("style/name", "contemporary").toString();
        QStringList keys = QStyleFactory::keys();
        for (QString key : keys) {
            ui->systemWidgetTheme->addItem(key);
            ui->systemWidgetTheme->setItemData(ui->systemWidgetTheme->count() - 1, key);

            if (key.toLower() == currentWidgetTheme.toLower()) {
                ui->systemWidgetTheme->setCurrentIndex(ui->systemWidgetTheme->count() - 1);
            }
        }
        ui->systemWidgetTheme->blockSignals(false);
    }

    //Load GTK3 themes into GTK3 theme box
    {
        ui->systemGTK3Theme->blockSignals(true);
        QString currentWidgetTheme = gtk3Settings->value("Settings/gtk-theme-name", "Contemporary").toString();
        QStringList themes;

        QString themeSearchDirectory = "/usr/share/themes/";

        QDir themeDir(themeSearchDirectory);
        QStringList themeDirEntries = themeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (QString entry : themeDirEntries) {
            if (QDir(themeSearchDirectory + entry + "/gtk-3.0/").exists()) {
                themes.append(entry);
            }
        }

        for (QString theme : themes) {
            ui->systemGTK3Theme->addItem(theme);
            ui->systemGTK3Theme->setItemData(ui->systemGTK3Theme->count() - 1, theme);

            if (theme.toLower() == currentWidgetTheme.toLower()) {
                ui->systemGTK3Theme->setCurrentIndex(ui->systemGTK3Theme->count() - 1);
            }
        }

        ui->systemGTK3Theme->blockSignals(false);
    }

    QString redshiftStart = settings.value("display/redshiftStart", "").toString();
    if (redshiftStart == "") {
        redshiftStart = ui->startRedshift->time().toString();
        settings.setValue("display/redshiftStart", redshiftStart);
    }
    ui->startRedshift->setTime(QTime::fromString(redshiftStart));

    QString redshiftEnd = settings.value("display/redshiftEnd", "").toString();
    if (redshiftEnd == "") {
        redshiftEnd = ui->endRedshift->time().toString();
        settings.setValue("display/redshiftEnd", redshiftEnd);
    }
    ui->endRedshift->setTime(QTime::fromString(redshiftEnd));

    QString redshiftVal = settings.value("display/redshiftIntensity", "").toString();
    if (redshiftVal == "") {
        redshiftVal = ui->endRedshift->time().toString();
        settings.setValue("display/redshiftIntensity", redshiftVal);
    }
    ui->redshiftIntensity->setValue(redshiftVal.toInt());

    if (settings.value("ui/useFullScreenEndSession", false).toBool()) {
        ui->endSessionConfirmFullScreen->setChecked(true);
        ui->endSessionConfirmInMenu->setChecked(false);
    } else {
        ui->endSessionConfirmFullScreen->setChecked(false);
        ui->endSessionConfirmInMenu->setChecked(true);
    }

    if (settings.contains("notifications/lockScreen")) {
        if (settings.value("notifications/lockScreen").toString() == "contents") {
            ui->showNotificationsContents->setChecked(true);
        } else if (settings.value("notifications/lockScreen").toString() == "none") {
            ui->showNotificationsNo->setChecked(true);
        } else {
            ui->showNotificationsOnly->setChecked(true);
        }
    } else {
        ui->showNotificationsOnly->setChecked(true);
    }

    QString themeType = themeSettings->value("color/type", "dark").toString();
    if (themeType == "light") {
        ui->lightColorThemeRadio->setChecked(true);
    } else if (themeType == "dark") {
        ui->darkColorThemeRadio->setChecked(true);
    } else if (themeType == "gray") {
        ui->grayColorThemeRadio->setChecked(true);
    } else if (themeType == "decorative") {
        ui->decorativeColorThemeRadio->setChecked(true);
    }

    int dpi = settings.value("screen/dpi", 96).toInt();
    switch (dpi) {
        case 96:
            ui->dpi100->setChecked(true);
            break;
        case 144:
            ui->dpi150->setChecked(true);
            break;
        case 192:
            ui->dpi200->setChecked(true);
            break;
        case 288:
            ui->dpi300->setChecked(true);
            break;
    }

    //Populate the language box
    Internationalisation::fillLanguageBox(ui->localeList);

    ui->lockScreenBackground->setText(lockScreenSettings->value("background", "/usr/share/tsscreenlock/triangles.svg").toString());
    //ui->lineEdit_2->setText(settings.value("startup/autostart", "").toString());
    ui->redshiftPause->setChecked(!settings.value("display/redshiftPaused", true).toBool());
    ui->TouchFeedbackSwitch->setChecked(settings.value("input/touchFeedbackSound", false).toBool());
    ui->SuperkeyGatewaySwitch->setChecked(settings.value("input/superkeyGateway", true).toBool());
    ui->TextSwitch->setChecked(settings.value("bar/showText", true).toBool());
    ui->windowManager->setText(settings.value("startup/WindowManagerCommand", "kwin_x11").toString());
    ui->barDesktopsSwitch->setChecked(settings.value("bar/showWindowsFromOtherDesktops", true).toBool());
    ui->MediaSwitch->setChecked(settings.value("notifications/mediaInsert", true).toBool());
    ui->StatusBarSwitch->setChecked(settings.value("bar/statusBar", false).toBool());
    ui->TouchInputSwitch->setChecked(settings.value("input/touch", false).toBool());
    ui->SuspendLockScreen->setChecked(settings.value("lockScreen/showOnSuspend", true).toBool());
    ui->LargeTextSwitch->setChecked(themeSettings->value("accessibility/largeText", false).toBool());
    ui->HighContrastSwitch->setChecked(themeSettings->value("accessibility/highcontrast", false).toBool());
    ui->systemAnimationsAccessibilitySwitch->setChecked(themeSettings->value("accessibility/systemAnimations", true).toBool());
    ui->CapsNumLockBellSwitch->setChecked(themeSettings->value("accessibility/bellOnCapsNumLock", false).toBool());
    ui->TwentyFourHourSwitch->setChecked(settings.value("time/use24hour", true).toBool());
    ui->AttenuateSwitch->setChecked(settings.value("notifications/attenuate", true).toBool());
    ui->BarOnBottom->setChecked(!settings.value("bar/onTop", true).toBool());
    ui->AutoShowBarSwitch->setChecked(settings.value("bar/autoshow", true).toBool());
    ui->SoundFeedbackSoundSwitch->setChecked(settings.value("sound/feedbackSound", true).toBool());
    ui->VolumeOverdriveSwitch->setChecked(settings.value("sound/volumeOverdrive", true).toBool());
    ui->batteryScreenOff->setValue(settings.value("power/batteryScreenOff", 15).toInt());
    ui->batterySuspend->setValue(settings.value("power/batterySuspend", 30).toInt());
    ui->powerScreenOff->setValue(settings.value("power/powerScreenOff", 30).toInt());
    ui->powerSuspend->setValue(settings.value("power/powerSuspend", 90).toInt());
    ui->sunlightRedshift->setChecked(settings.value("display/redshiftSunlightCycle", false).toBool());
    ui->EmphasiseAppSwitch->setChecked(settings.value("notifications/emphasiseApp", true).toBool());
    ui->CompactBarSwitch->setChecked(settings.value("bar/compact", false).toBool());
    ui->LocationMasterSwitch->setChecked(locationSettings->value("master/master", true).toBool());
    ui->powerButtonPressed->setCurrentIndex(settings.value("power/onPowerButtonPressed", 0).toInt());
    updateAccentColourBox();
    updateRedshiftTime();
    on_StatusBarSwitch_toggled(ui->StatusBarSwitch->isChecked());

    if (QFile(QDir::homePath() + "/.theshell/mousepassword").exists()) {
        ui->removeMousePassword->setVisible(true);
    } else {
        ui->removeMousePassword->setVisible(false);
    }

    QString defaultFont;
    if (QFontDatabase().families().contains("Contemporary")) {
        defaultFont = "Contemporary";
    } else {
        defaultFont = "Noto Sans";
    }
    ui->systemFont->setCurrentFont(QFont(themeSettings->value("fonts/defaultFamily", defaultFont).toString(), themeSettings->value("font/defaultSize", 10).toInt()));
    ui->systemFontSize->setValue(themeSettings->value("font/defaultSize", 10).toInt());

    QString gtk3FontString = gtk3Settings->value("Settings/gtk-font-name", "Contemporary 10").toString();
    QString gtk3FontFamily = gtk3FontString.left(gtk3FontString.lastIndexOf(" "));
    QString gtk3FontSize = gtk3FontString.mid(gtk3FontString.lastIndexOf(" ") + 1);
    ui->systemGTK3Font->setCurrentFont(QFont(gtk3FontFamily, gtk3FontSize.toInt()));
    ui->systemGTK3FontSize->setValue(gtk3FontSize.toInt());

    switch (settings.value("power/suspendMode", 0).toInt()) {
        case 0:
            ui->powerSuspendNormally->setChecked(true);
            break;
        case 1:
            ui->powerSuspendTurnOffScreen->setChecked(true);
            break;
        case 2:
            ui->powerSuspendHibernate->setChecked(true);
            break;
    }

    eventTimer = new QTimer(this);
    eventTimer->setInterval(1000);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(processTimer()));
    eventTimer->start();

    networkCheckTimer = new QTimer(this);
    networkCheckTimer->setInterval(60000);
    connect(networkCheckTimer, SIGNAL(timeout()), this, SLOT(doNetworkCheck()));
    networkCheckTimer->start();
    doNetworkCheck();

    QObjectList allObjects;
    allObjects.append(this);

    ui->notificationSoundBox->blockSignals(true);
    ui->notificationSoundBox->addItem("Triple Ping");
    ui->notificationSoundBox->addItem("Upside Down");
    ui->notificationSoundBox->addItem("Echo");

    QString notificationSound = settings.value("notifications/sound", "tripleping").toString();
    if (notificationSound == "tripleping") {
        ui->notificationSoundBox->setCurrentIndex(0);
    } else if (notificationSound == "upsidedown") {
        ui->notificationSoundBox->setCurrentIndex(1);
    } else if (notificationSound == "echo") {
        ui->notificationSoundBox->setCurrentIndex(2);
    }
    ui->notificationSoundBox->blockSignals(false);

    //Don't forget to change settings pane setup things
    ui->settingsList->item(ui->settingsList->count() - 1)->setSelected(true);
    ui->settingsTabs->setCurrentIndex(ui->settingsTabs->count() - 1);

    //Get distribution information
    {
        QString osreleaseFile = "";
        if (QFile("/etc/os-release").exists()) {
            osreleaseFile = "/etc/os-release";
        } else if (QFile("/usr/lib/os-release").exists()) {
            osreleaseFile = "/usr/lib/os-release";
        }

        if (osreleaseFile != "") {
            QFile information(osreleaseFile);
            information.open(QFile::ReadOnly);
            while (!information.atEnd()) {
                QString line = information.readLine();
                if (line.startsWith("pretty_name=", Qt::CaseInsensitive)) {
                    ui->distroName->setText(line.remove("pretty_name=", Qt::CaseInsensitive).remove("\"").remove("\n"));
                } else if (line.startsWith("home_url=", Qt::CaseInsensitive)) {
                    ui->distroWebpage->setText(line.remove("home_url=", Qt::CaseInsensitive).remove("\"").remove("\n"));
                } else if (line.startsWith("support_url=", Qt::CaseInsensitive)) {
                    ui->distroSupport->setText(line.remove("support_url=", Qt::CaseInsensitive).remove("\"").remove("\n"));
                }
            }
            information.close();
        }

        struct sysinfo* info = new struct sysinfo;
        if (sysinfo(info) == 0) {
            ui->availableMemory->setText(calculateSize(info->totalram));
            ui->availableSwap->setText(calculateSize(info->totalswap));
        } else {

        }
        delete info;

        ui->kernelVersion->setText(QSysInfo::kernelVersion());
        ui->qtVersion->setText(qVersion());

        QFile cpuInfoFile("/proc/cpuinfo");
        cpuInfoFile.open(QFile::ReadOnly);
        QByteArray cpuInfoBuf = cpuInfoFile.readAll();
        QBuffer cpuInfo(&cpuInfoBuf);
        cpuInfo.open(QBuffer::ReadOnly);
        QStringList knownCpus;
        while (!cpuInfo.atEnd()) {
            QString line = cpuInfo.readLine();
            if (line.startsWith("model name")) {
                QString cpu = line.mid(line.indexOf(":") + 1).trimmed();
                knownCpus.append(cpu);
            }
        }

        QStringList shownCpus;
        while (knownCpus.length() > 0) {
            QString cpu = knownCpus.value(0);
            int numberOfThisCpu = knownCpus.count(cpu);

            knownCpus.removeAll(cpu);

            if (numberOfThisCpu == 1) {
                shownCpus.append(cpu);
            } else {
                shownCpus.append(QString::number(numberOfThisCpu) + " × " + cpu);
            }
        }

        if (shownCpus.length() == 0) {
            ui->processorType->setText(tr("Unknown"));
        } else {
            ui->processorType->setText(shownCpus.join(" · "));
        }
    }

    #ifdef BLUEPRINT
        ui->tsVersion->setText(tr("theShell %1 - Blueprint").arg(TS_VERSION));
        ui->compileDate->setText(tr("You compiled theShell on %1").arg(__DATE__));
    #else
        ui->tsVersion->setText(tr("theShell %1").arg(TS_VERSION));
        ui->compileDate->setVisible(false);
    #endif

    //Update keyboard layouts
    struct KeyboardLayoutReturn {
        QMap<QString, QString> availableKeyboardLayout;
        QStringList currentKeyboardLayout;
    };
    QFuture<KeyboardLayoutReturn> keyboardLayouts = QtConcurrent::run([=]() -> KeyboardLayoutReturn {
        KeyboardLayoutReturn retval;

        QDir xkbLayouts("/usr/share/X11/xkb/symbols");
        for (QFileInfo layoutInfo : xkbLayouts.entryInfoList()) {
            if (layoutInfo.isDir()) continue;

            QString layout = layoutInfo.baseName();
            QFile file(layoutInfo.filePath());
            file.open(QFile::ReadOnly);

            QString currentSubLayout = "";
            while (!file.atEnd()) {
                QString line = file.readLine().trimmed();
                if (line.startsWith("xkb_symbols") && line.endsWith("{")) {
                    QRegExp lineRx("\".+\"");
                    lineRx.indexIn(line);

                    if (lineRx.capturedTexts().count() != 0) {
                        currentSubLayout = lineRx.capturedTexts().first().remove("\"");
                    } else {
                        currentSubLayout = "";
                    }
                } else if (line.startsWith("name")) {
                    QRegExp lineRx("\".+\"");
                    lineRx.indexIn(line);

                    if (lineRx.capturedTexts().count() != 0 && currentSubLayout != "") {
                        retval.availableKeyboardLayout.insert(layout + "(" + currentSubLayout + ")", lineRx.capturedTexts().first().remove("\""));
                    } else {
                        currentSubLayout = "";
                    }
                }
            }

            file.close();
        }

        retval.currentKeyboardLayout = settings.value("input/layout", "us(basic)").toString().split(",");
        return retval;
    });
    QFutureWatcher<KeyboardLayoutReturn>* keyboardLayoutWatcher = new QFutureWatcher<KeyboardLayoutReturn>();
    connect(keyboardLayoutWatcher, &QFutureWatcher<KeyboardLayoutReturn>::finished, [=] {
        keyboardLayoutWatcher->deleteLater();

        KeyboardLayoutReturn layouts = keyboardLayouts.result();
        for (QString key : layouts.availableKeyboardLayout.keys()) {
            QString value = layouts.availableKeyboardLayout.value(key);

            QListWidgetItem* item = new QListWidgetItem();
            item->setText(value);
            item->setData(Qt::UserRole, key);
            ui->availableKeyboardLayouts->addItem(item);
        }

        for (QString layout : layouts.currentKeyboardLayout) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(layouts.availableKeyboardLayout.value(layout, layout));
            item->setData(Qt::UserRole, layout);
            ui->selectedLayouts->addItem(item);
        }

        ui->availableKeyboardLayouts->sortItems();
        connect(ui->selectedLayouts->model(), SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SLOT(KeyboardLayoutsMoved()));
        setKeyboardLayout(settings.value("input/currentLayout", "us(basic)").toString());
    });
    keyboardLayoutWatcher->setFuture(keyboardLayouts);

    //Set up timer ringtones
    ringtone = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    connect(AudioMan, &AudioManager::QuietModeChanged, [=](AudioManager::quietMode mode) {
        ui->quietModeSound->setChecked(false);
        ui->quietModeCriticalOnly->setChecked(false);
        ui->quietModeNotification->setChecked(false);
        ui->quietModeMute->setChecked(false);
        ui->quietModeForeverButton->setEnabled(true);
        ui->quietModeTurnOffAt->setEnabled(true);
        ui->quietModeTurnOffIn->setEnabled(true);

        if (mode == AudioManager::none) {
            ui->quietModeSound->setChecked(true);

            ui->quietModeForeverButton->setEnabled(false);
            ui->quietModeTurnOffAt->setEnabled(false);
            ui->quietModeTurnOffIn->setEnabled(false);
            ui->quietModeForeverButton->setChecked(true);
        } else if (mode == AudioManager::notifications) {
            ui->quietModeNotification->setChecked(true);
        } else if (mode == AudioManager::critical) {
            ui->quietModeCriticalOnly->setChecked(true);
        } else {
            ui->quietModeMute->setChecked(true);
        }
        ui->quietModeDescription->setText(AudioMan->getCurrentQuietModeDescription());
    });
    ui->quietModeForeverButton->setChecked(true);
    ui->quietModeForeverButton->setEnabled(false);
    ui->quietModeTurnOffAt->setEnabled(false);
    ui->quietModeTurnOffIn->setEnabled(false);
    ui->quietModeDescription->setText(AudioMan->getCurrentQuietModeDescription());

    slice1.setStartValue((float) (this->width() - 250 * getDPIScaling()));
    slice1.setEndValue((float) (this->width() - 300 * getDPIScaling()));
    slice1.setEasingCurve(QEasingCurve::OutCubic);
    slice1.setDuration(15000);
    connect(&slice1, &tVariantAnimation::finished, [=] {
        slice1.setStartValue(slice1.endValue());
        if (slice1.endValue() == this->width() - 300 * getDPIScaling()) {
            slice1.setEndValue(this->width() - 250 * getDPIScaling());
        } else {
            slice1.setEndValue(this->width() - 300 * getDPIScaling());
        }
        slice1.setEasingCurve(QEasingCurve::InOutCubic);
        slice1.start();
    });
    connect(&slice1, SIGNAL(valueChanged(QVariant)), ui->partFrame, SLOT(repaint()));
    slice1.start();

    QTimer::singleShot(2500, [=] {
        slice2.setStartValue((float) (this->width() - 300 * getDPIScaling()));
        slice2.setEndValue((float) (this->width() - 350 * getDPIScaling()));
        slice2.setEasingCurve(QEasingCurve::OutCubic);
        slice2.setDuration(15000);
        connect(&slice2, &tVariantAnimation::finished, [=] {
            slice2.setStartValue(slice2.endValue());
            if (slice2.endValue() == this->width() - 350 * getDPIScaling()) {
                slice2.setEndValue(this->width() - 300 * getDPIScaling());
            } else {
                slice2.setEndValue(this->width() - 350 * getDPIScaling());
            }
            slice2.setEasingCurve(QEasingCurve::InOutCubic);
            slice2.start();
        });
        slice2.start();
    });

    //Load plugins
    QList<QDir> searchDirs;
    QJsonArray errors;
    searchDirs.append(QApplication::applicationDirPath() + "/../statuscenter/");
    searchDirs.append(QApplication::applicationDirPath() + "/../daemons/");
    #ifdef BLUEPRINT
        searchDirs.append(QDir("/usr/lib/theshellb/panes"));
        searchDirs.append(QDir("/usr/lib/theshellb/daemons"));
    #else
        searchDirs.append(QDir("/usr/lib/theshell/panes"));
        searchDirs.append(QDir("/usr/lib/theshell/daemons"));
    #endif
    QStringList loadedPanes, loadedSettings;
    for (QDir pluginsDir : searchDirs) {
        QDirIterator pluginsIterator(pluginsDir, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

        while (pluginsIterator.hasNext()) {
            pluginsIterator.next();
            if (pluginsIterator.fileInfo().suffix() != "so" && pluginsIterator.fileInfo().suffix() != "a") continue;
            QPluginLoader loader(pluginsIterator.filePath());
            QJsonObject metadata = loader.metaData();
            if (!metadata.contains("name")) {
                metadata.insert("name", pluginsIterator.fileName());
            }

            QObject* plugin = loader.instance();
            if (plugin == nullptr) {
                metadata.insert("error", loader.errorString());
                errors.append(metadata);
            } else {
                StatusCenterPane* p = qobject_cast<StatusCenterPane*>(plugin);
                if (p) {
                    qDebug() << "Loading" << metadata.value("name").toString();
                    p->loadLanguage(QLocale().name());
                    for (StatusCenterPaneObject* pane : p->availablePanes()) {
                        if (pane->name() == "Overview" && pane->type().testFlag(StatusCenterPaneObject::Informational)) {
                            if (!loadedPanes.contains("Overview")) {
                                //Special handling for Overview pane
                                overviewFrame = pane->mainWidget();
                                overviewFrame->setAutoFillBackground(true);
                                ui->pageStack->insertWidget(0, overviewFrame);

                                loadedPanes.append(pane->name());
                            }
                        } else {
                            if (pane->type().testFlag(StatusCenterPaneObject::Informational)) {
                                if (!loadedPanes.contains(pane->name())) {
                                    ClickableLabel* label = new ClickableLabel(this);
                                    label->setText(pane->name());
                                    ui->InformationalPluginsLayout->addWidget(label);

                                    ui->pageStack->insertWidget(ui->pageStack->count() - 1, pane->mainWidget());
                                    pane->mainWidget()->setAutoFillBackground(true);

                                    connect(label, &ClickableLabel::clicked, [=] {
                                        changeDropDown(pane->mainWidget(), label);
                                        if (ui->lightColorThemeRadio->isChecked()) {
                                            setHeaderColour(pane->informationalAttributes.lightColor);
                                        } else {
                                            setHeaderColour(pane->informationalAttributes.darkColor);
                                        }
                                    });

                                    loadedPanes.append(pane->name());
                                }
                            }

                            if (pane->type().testFlag(StatusCenterPaneObject::Setting)) {
                                if (!loadedSettings.contains(pane->name())) {
                                    QListWidgetItem* item = new QListWidgetItem();
                                    item->setText(pane->name());
                                    item->setIcon(pane->settingAttributes.icon);
                                    item->setData(Qt::UserRole, -1);
                                    ui->settingsList->insertItem(ui->settingsList->count() - 3, item);

                                    if (pane->settingAttributes.menuWidget != nullptr) {
                                        int settingNumber = ui->settingsListStack->addWidget(pane->settingAttributes.menuWidget);
                                        pane->settingAttributes.menuWidget->setAutoFillBackground(true);
                                        item->setData(Qt::UserRole, settingNumber);
                                    }

                                    ui->settingsTabs->insertWidget(ui->settingsTabs->count() - 3, pane->mainWidget());
                                    pane->mainWidget()->setAutoFillBackground(true);

                                    loadedSettings.append(pane->name());
                                }
                            }
                        }

                        pane->sendMessage = [=](QString message, QVariantList args) {
                            this->pluginMessage(message, args, pane);
                        };
                        pluginObjects.insert(pane->mainWidget(), pane);
                    }
                    loadedPlugins.append(p);
                }
            }
        }
    }

    if (!loadedPanes.contains("Overview")) {
        qWarning() << "Could not load Overview pane";
        qWarning() << "theShell may not work properly";
    }

    if (errors.count() == 0) {
        ui->settingsTabs->removeWidget(ui->UnavailablePanesPage);
        delete ui->settingsList->takeItem(ui->settingsList->count() - 3);
    } else {
        ui->settingsUnavailableTable->setColumnCount(2);
        ui->settingsUnavailableTable->setRowCount(errors.count());
        ui->settingsUnavailableTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Reason");
        ui->settingsUnavailableTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->settingsUnavailableTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        for (int i = 0; i < errors.count(); i++) {
            QJsonObject o = errors.at(i).toObject();
            ui->settingsUnavailableTable->setItem(i, 0, new QTableWidgetItem(o.value("name").toString()));
            ui->settingsUnavailableTable->setItem(i, 1, new QTableWidgetItem(o.value("error").toString()));
        }
    }

    QScroller::grabGesture(ui->settingsList, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->availableKeyboardLayouts, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->timezoneCityList, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->timezoneList, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->appsGraph, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(ui->autostartAppList, QScroller::LeftMouseButtonGesture);

    connect(tVirtualKeyboard::instance(), &tVirtualKeyboard::keyboardVisibleChanged, [=](bool visible) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        if (visible) {
            this->setFixedHeight(screenGeometry.height() - tVirtualKeyboard::instance()->height());
        } else {
            this->setFixedHeight(screenGeometry.height() + 1);
        }
    });

    updateStruts();
    updateAutostart();

    /*QTimer::singleShot(5000, [=] {
        this->setProperty("aw", 0);
        QTimer* timer = new QTimer();
        timer->setInterval(1000);
        connect(timer, &QTimer::timeout, [=] {
            int number = this->property("aw").toInt();
            number++;

            if (number == 100) {
                timer->stop();
                emit statusBarProgressFinished("Copied", "lirios-2018-04-28.iso");
            } else {
                emit statusBarProgress("Copying", "lirios-2018.04.28.iso", number);
            }

            this->setProperty("aw", number);
        });
        timer->start();
    });*/
}

InfoPaneDropdown::~InfoPaneDropdown()
{
    delete ui;
}

InfoPaneNotOnTopLocker::InfoPaneNotOnTopLocker(InfoPaneDropdown *infoPane) {
    this->infoPane = infoPane;
    infoPane->setWindowFlags(Qt::FramelessWindowHint);
}

InfoPaneNotOnTopLocker::~InfoPaneNotOnTopLocker() {
    infoPane->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    infoPane->showNoAnimation();
}

void InfoPaneDropdown::DBusServiceRegistered(QString serviceName) {
    if (serviceName == "org.thesuite.tsbt") {
        QDBusInterface interface("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", QDBusConnection::sessionBus());
        ui->BluetoothSwitch->setEnabled(true);
        ui->BluetoothSwitch->setChecked(interface.property("BluetoothEnabled").toBool());
    }
}

void InfoPaneDropdown::DBusServiceUnregistered(QString serviceName) {
    if (serviceName == "org.thesuite.tsbt") {
        ui->BluetoothSwitch->setEnabled(false);
    }
}

void InfoPaneDropdown::bluetoothEnabledChanged() {
    ui->BluetoothSwitch->setChecked(QDBusInterface("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", QDBusConnection::sessionBus()).property("BluetoothEnabled").toBool());
}

void InfoPaneDropdown::newNetworkDevice(QDBusObjectPath device) {
    QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus(), this);
    if (i->property("DeviceType").toInt() == 2) { //WiFi Device
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wireless", "AccessPointAdded", this, SLOT(getNetworks()));
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wireless", "AccessPointRemoved", this, SLOT(getNetworks()));
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", "StateChanged", this, SLOT(getNetworks()));
    } else if (i->property("DeviceType").toInt() == 1) { //Wired Device
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wired", "PropertiesChanged", this, SLOT(getNetworks()));
    }
    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(getNetworks()));

    QDBusInterface *stats = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Statistics", QDBusConnection::systemBus(), this);
    stats->setProperty("RefreshRateMs", (uint) 1000);
    getNetworks();
    stats->deleteLater();
    i->deleteLater();
}

void InfoPaneDropdown::on_WifiSwitch_toggled(bool checked)
{
    QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
    if (i->property("WirelessEnabled").toBool() != checked) {
        i->setProperty("WirelessEnabled", checked);
    }

    if (i->property("WirelessEnabled").toBool()) {
        ui->WifiSwitch->setChecked(true);
    } else {
        ui->WifiSwitch->setChecked(false);
    }

    i->deleteLater();
}

void InfoPaneDropdown::processTimer() {
    QTime time = QTime::currentTime();
    {
        int currentMsecs = time.msecsSinceStartOfDay();
        int startMsecs = ui->startRedshift->time().msecsSinceStartOfDay();
        int endMsecs = ui->endRedshift->time().msecsSinceStartOfDay();
        int endIntensity = ui->redshiftIntensity->value();
        const int oneHour = 3600000;
        QString redshiftCommand;

        if (ui->redshiftPause->isChecked()) {
            //Calculate redshift value
            //Transition to redshift is 1 hour from the start.

            int intensity;
            if (startMsecs > endMsecs) { //Start time is later then end time
                if (currentMsecs < endMsecs || currentMsecs > startMsecs) {
                    intensity = endIntensity;
                } else if (currentMsecs < startMsecs && currentMsecs > startMsecs - oneHour) {
                    int timeFrom = currentMsecs - (startMsecs - oneHour);
                    float percentage = ((float) timeFrom / (float) oneHour);
                    int progress = (6500 - endIntensity) * percentage;
                    intensity = 6500 - progress;
                } else if (currentMsecs > endMsecs && currentMsecs < endMsecs + oneHour) {
                    int timeFrom = endMsecs - (currentMsecs - oneHour);
                    float percentage = ((float) timeFrom / (float) oneHour);
                    int progress = (6500 - endIntensity) * percentage;
                    intensity = 6500 - progress;
                } else {
                    intensity = 6500;
                }
            } else { //Start time is earlier then end time
                if (currentMsecs < endMsecs && currentMsecs > startMsecs) {
                    intensity = endIntensity;
                } else if (currentMsecs < startMsecs && currentMsecs > startMsecs - oneHour) {
                    int timeFrom = currentMsecs - (startMsecs - oneHour);
                    float percentage = ((float) timeFrom / (float) oneHour);
                    int progress = (6500 - endIntensity) * percentage;
                    intensity = 6500 - progress;
                } else if (currentMsecs > endMsecs && currentMsecs < endMsecs + oneHour) {
                    int timeFrom = endMsecs - (currentMsecs - oneHour);
                    float percentage = ((float) timeFrom / (float) oneHour);
                    int progress = (6500 - endIntensity) * percentage;
                    intensity = 6500 - progress;
                } else {
                    intensity = 6500;
                }
            }

            //Check Redshift override
            if (overrideRedshift != 0) {
                if (intensity == 6500 && overrideRedshift == 1) {
                    overrideRedshift = 0; //Reset Redshift override
                } else if (intensity != 6500 && overrideRedshift == 2) {
                    overrideRedshift = 0; //Reset Redshift override
                } else {
                    if (overrideRedshift == 1) {
                        intensity = 6500;
                    } else {
                        intensity = endIntensity;
                    }
                }
            }

            redshiftCommand = "redshift -O " + QString::number(intensity);

            isRedshiftOn = true;
            if (intensity == 6500 && effectiveRedshiftOn) {
                effectiveRedshiftOn = false;
                ui->redshiftSwitch->setChecked(false);
                emit redshiftEnabledChanged(false);
            } else if (intensity != 6500 && !effectiveRedshiftOn) {
                effectiveRedshiftOn = true;
                ui->redshiftSwitch->setChecked(true);
                emit redshiftEnabledChanged(true);
            }
        } else {
            //Check Redshift Override
            if (overrideRedshift == 2) {
                redshiftCommand = "redshift -O " + QString::number(endIntensity);
            } else {
                redshiftCommand = "redshift -O 6500";
            }

            if (isRedshiftOn) {
                isRedshiftOn = false;
                effectiveRedshiftOn = false;
                ui->redshiftSwitch->setChecked(false);
                emit redshiftEnabledChanged(false);
            }
        }

        if (isNewRedshift) {
            redshiftCommand += " -P";
        }

        QProcess* redshiftAdjust = new QProcess();
        redshiftAdjust->start(redshiftCommand);
        connect(redshiftAdjust, SIGNAL(finished(int)), redshiftAdjust, SLOT(deleteLater()));
    }

    /*{
        cups_dest_t *destinations;
        int destinationCount = cupsGetDests(&destinations);

        for (int i = 0; i < destinationCount; i++) {
            cups_dest_t currentDestination = destinations[i];

            if (!printersFrames.keys().contains(currentDestination.name)) {
                QFrame* frame = new QFrame();
                QHBoxLayout* layout = new QHBoxLayout();
                layout->setMargin(0);
                frame->setLayout(layout);

                QFrame* statFrame = new QFrame();
                QHBoxLayout* statLayout = new QHBoxLayout();
                statLayout->setMargin(0);
                statFrame->setLayout(statLayout);
                layout->addWidget(statFrame);

                QLabel* iconLabel = new QLabel();
                QPixmap icon = QIcon::fromTheme("printer").pixmap(22 * getDPIScaling(), 22 * getDPIScaling());
                if (currentDestination.is_default) {
                    QPainter *p = new QPainter();
                    p->begin(&icon);
                    p->drawPixmap(10 * getDPIScaling(), 10 * getDPIScaling(), 12 * getDPIScaling(), 12 * getDPIScaling(), QIcon::fromTheme("emblem-checked").pixmap(12 * getDPIScaling(), 12 * getDPIScaling()));
                    p->end();
                }
                iconLabel->setPixmap(icon);
                statLayout->addWidget(iconLabel);

                QLabel* nameLabel = new QLabel();
                nameLabel->setText(currentDestination.name);
                QFont font = nameLabel->font();
                font.setBold(true);
                nameLabel->setFont(font);
                statLayout->addWidget(nameLabel);

                QLabel* statLabel = new QLabel();
                statLabel->setText(tr("Idle"));
                statLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                statLayout->addWidget(statLabel);

                /*QPushButton* button = new QPushButton();
                button->setIcon(QIcon::fromTheme("window-close"));
                connect(button, &QPushButton::clicked, [=]() {
                    emit closeNotification(id);
                });
                layout->addWidget(button);

                ui->printersList->layout()->addWidget(frame);
                printersFrames.insert(currentDestination.name, frame);
                printersStatFrames.insert(currentDestination.name, frame);
                printersStats.insert(currentDestination.name, statLabel);
            }

            QString state = "";
            QString stateReasons = "";
            for (int i = 0; i < currentDestination.num_options; i++) {
                cups_option_t currentOption = currentDestination.options[i];

                if (strncmp(currentOption.name, "printer-state", strlen(currentOption.name)) == 0) {
                    if (strncmp(currentOption.value, "3", 1) == 0) {
                        state = tr("Idle");
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "4", 1) == 0) {
                        state = tr("Printing");
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "5", 1) == 0) {
                        state = tr("Stopped");
                        printersStatFrames.value(currentDestination.name)->setEnabled(false);
                    }
                } else if (strncmp(currentOption.name, "printer-state-reasons", strlen(currentOption.name)) == 0) {
                    stateReasons = QString::fromUtf8(currentOption.value, strlen(currentOption.value));
                }
            }
            printersStats.value(currentDestination.name)->setText(state + " / " + stateReasons);

        }

        cupsFreeDests(destinationCount, destinations);
    }*/
}

void InfoPaneDropdown::show(dropdownType showWith) {
    changeDropDown(showWith, false);
    if (!this->isVisible()) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();

        if (settings.value("bar/onTop", true).toBool()) {
            this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), screenGeometry.width(), screenGeometry.height() + 1);
        } else {
            this->setGeometry(screenGeometry.x(), screenGeometry.bottom(), screenGeometry.width(), screenGeometry.height() + 1);
        }

        Atom DesktopWindowTypeAtom;
        DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False);
        XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                         XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

        unsigned long desktop = 0xFFFFFFFF;
        XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                         XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

        QDialog::show();
        this->setFixedWidth(screenGeometry.width());

        if (tVirtualKeyboard::instance()->keyboardVisible()) {
            this->setFixedHeight(screenGeometry.height() - tVirtualKeyboard::instance()->height());
        } else {
            this->setFixedHeight(screenGeometry.height() + 1);
        }

        if (settings.value("bar/onTop", true).toBool()) {
            previousDragY = -1;
        } else {
            previousDragY = screenGeometry.bottom();
        }
        completeDragDown();
    }

    //Get Current Brightness
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -get");
    backlight->waitForFinished();
    float output = ceil(QString(backlight->readAll()).toFloat());
    delete backlight;

    ui->brightnessSlider->setValue((int) output);
}

void InfoPaneDropdown::showNoAnimation() {
    QDialog::show();
    previousDragY = -1;
    completeDragDown();
}

void InfoPaneDropdown::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());

    if (settings.value("bar/onTop", true).toBool()) {
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - screenGeometry.height() + 1, this->width(), this->height()));
    } else {
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.bottom() + 1, this->width(), this->height()));
    }
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, &tPropertyAnimation::finished, [=] {
        for (StatusCenterPaneObject* plugin : pluginObjects.values()) {
            plugin->message("hide");
            plugin->showing = false;
        }
        QDialog::hide();
    });
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();

    if (ui->quietModeExtras->height() != 0) {
        ui->quietModeExpandButton->click();
    }
}

void InfoPaneDropdown::changeDropDown(dropdownType changeTo, bool doAnimation) {
    //Switch to the requested frame
    switch (changeTo) {
        case Clock:
            changeDropDown(overviewFrame, ui->clockLabel, doAnimation);

            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(0, 150, 0));
            } else {
                setHeaderColour(QColor(0, 50, 0));
            }
            break;
        case Battery:
            changeDropDown(ui->statusFrame, ui->batteryLabel, doAnimation);
            updateBatteryChart();
            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(200, 150, 0));
            } else {
                setHeaderColour(QColor(100, 50, 0));
            }
            break;
        case Notifications:
            changeDropDown(ui->notificationsFrame, ui->notificationsLabel, doAnimation);
            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(200, 50, 100));
            } else {
                setHeaderColour(QColor(100, 25, 50));
            }
            break;
        case Network:
            changeDropDown(ui->networkFrame, ui->networkLabel, doAnimation);
            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(100, 100, 255));
            } else {
                setHeaderColour(QColor(50, 50, 100));
            }
            break;
        case KDEConnect:
            changeDropDown(ui->kdeConnectFrame, ui->kdeconnectLabel, doAnimation);
            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(200, 100, 255));
            } else {
                setHeaderColour(QColor(50, 0, 100));
            }
            break;
        case Settings:
            changeDropDown(ui->settingsFrame, nullptr, doAnimation);
            if (ui->lightColorThemeRadio->isChecked()) {
                setHeaderColour(QColor(0, 150, 255));
            } else {
                setHeaderColour(QColor(0, 50, 100));
            }
            break;
    }
}

void InfoPaneDropdown::changeDropDown(QWidget *changeTo, ClickableLabel* label, bool doAnimation) {
    for (StatusCenterPaneObject* plugin : pluginObjects.values()) {
        plugin->message("hide");
        plugin->showing = false;
    }
    if (pluginObjects.contains(changeTo)) {
        pluginObjects.value(changeTo)->message("show");
        pluginObjects.value(changeTo)->showing = true;
    }

    //Switch to the requested frame
    if (ui->pageStack->currentWidget() == changeTo) return; //Do nothing

    ui->pageStack->setCurrentWidget(changeTo, doAnimation);
    if (changeTo == overviewFrame) {
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(0, 150, 0));
        } else {
            setHeaderColour(QColor(0, 50, 0));
        }
    } else if (changeTo == ui->statusFrame) {
        updateBatteryChart();
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(200, 150, 0));
        } else {
            setHeaderColour(QColor(100, 50, 0));
        }
    } else if (changeTo == ui->notificationsFrame) {
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(200, 50, 100));
        } else {
            setHeaderColour(QColor(100, 25, 50));
        }
    } else if (changeTo == ui->networkFrame) {
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(100, 100, 255));
        } else {
            setHeaderColour(QColor(50, 50, 100));
        }
    } else if (changeTo == ui->kdeConnectFrame) {
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(200, 100, 255));
        } else {
            setHeaderColour(QColor(50, 0, 100));
        }
    } else if (changeTo == ui->settingsFrame) {
        if (ui->lightColorThemeRadio->isChecked()) {
            setHeaderColour(QColor(0, 150, 255));
        } else {
            setHeaderColour(QColor(0, 50, 100));
        }
    }
    //Plugin'd panes are handled in the click handler for the label

    //Set the correct label
    for (int i = 0; i < ui->InformationalPluginsLayout->count(); i++) {
        ((ClickableLabel*) ui->InformationalPluginsLayout->itemAt(i)->widget())->setShowDisabled(true);
    }

    if (label != nullptr) {
        label->setShowDisabled(false);
    }

    //Determine the last frame
    int lastFrame = 0;
    for (int i = ui->pageStack->count() - 1; i >= 0; i--) {
        QWidget* w = ui->pageStack->widget(i);
        if (w != ui->settingsFrame) {
            lastFrame = i;
            break;
        }
    }

    if (changeTo == ui->settingsFrame) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(false);
    } else if (ui->pageStack->indexOf(changeTo) == 0) { //First item
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(true);
    } else if (ui->pageStack->indexOf(changeTo) == lastFrame) { //Last item
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(false);
    } else {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(true);
    }
}

void InfoPaneDropdown::on_pushButton_clicked()
{
    this->close();
}

void InfoPaneDropdown::getNetworks() {
    ui->NetworkManager->updateGlobals();
}

void InfoPaneDropdown::on_pushButton_5_clicked()
{
    int change = ui->pageStack->currentIndex() - 1;
    changeDropDown(ui->pageStack->widget(change), (ClickableLabel*) ui->InformationalPluginsLayout->itemAt(change)->widget());
}

void InfoPaneDropdown::on_pushButton_6_clicked()
{
    int change = ui->pageStack->currentIndex() + 1;
    changeDropDown(ui->pageStack->widget(change), (ClickableLabel*) ui->InformationalPluginsLayout->itemAt(change)->widget());
}

void InfoPaneDropdown::on_clockLabel_clicked()
{
    changeDropDown(Clock);
}

void InfoPaneDropdown::on_batteryLabel_clicked()
{
    changeDropDown(Battery);
}

void InfoPaneDropdown::on_networkLabel_clicked()
{
    changeDropDown(Network);
}

void InfoPaneDropdown::on_notificationsLabel_clicked()
{
    changeDropDown(Notifications);
}

void InfoPaneDropdown::on_pushButton_7_clicked()
{
    changeDropDown(Settings);
}

void InfoPaneDropdown::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void InfoPaneDropdown::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void InfoPaneDropdown::on_resolutionButton_clicked()
{
    QProcess::startDetached("kcmshell5 kcm_kscreen");
    this->close();
}

void InfoPaneDropdown::on_startRedshift_timeChanged(const QTime &time)
{
    settings.setValue("display/redshiftStart", time.toString());
    processTimer();
}

void InfoPaneDropdown::on_endRedshift_timeChanged(const QTime &time)
{
    settings.setValue("display/redshiftEnd", time.toString());
    processTimer();
}

void InfoPaneDropdown::on_redshiftIntensity_sliderMoved(int position)
{
    if (isNewRedshift) {
        QProcess::startDetached("redshift -P -O " + QString::number(position));
    } else {
        QProcess::startDetached("redshift -O " + QString::number(position));
    }
}

void InfoPaneDropdown::on_redshiftIntensity_sliderReleased()
{
    if (!isRedshiftOn) {
        if (isNewRedshift) {
            QProcess::startDetached("redshift -P -O 6500");
        } else {
            QProcess::startDetached("redshift -O 6500");
        }
    }
}

void InfoPaneDropdown::on_redshiftIntensity_valueChanged(int value)
{
    settings.setValue("display/redshiftIntensity", value);
}

void InfoPaneDropdown::newNotificationReceived(int id, QString summary, QString body, QIcon icon) {
    if (notificationFrames.keys().contains(id)) { //Notification already exists, update it.
        QFrame* frame = notificationFrames.value(id);
        frame->property("summaryLabel").value<QLabel*>()->setText(summary);
        frame->property("bodyLabel").value<QLabel*>()->setText(body);
    } else {
        QFrame* frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setMargin(0);
        frame->setLayout(layout);

        QLabel* iconLabel = new QLabel();
        iconLabel->setPixmap(icon.pixmap(22 * getDPIScaling(), 22 * getDPIScaling()));
        layout->addWidget(iconLabel);

        QLabel* sumLabel = new QLabel();
        sumLabel->setText(summary);
        QFont font = sumLabel->font();
        font.setBold(true);
        sumLabel->setFont(font);
        layout->addWidget(sumLabel);

        QLabel* bodyLabel = new QLabel();
        bodyLabel->setText(body);
        bodyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(bodyLabel);

        QLabel* dateLabel = new QLabel();
        dateLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
        layout->addWidget(dateLabel);

        QPushButton* button = new QPushButton();
        button->setIcon(QIcon::fromTheme("window-close"));
        connect(button, &QPushButton::clicked, [=]() {
            emit closeNotification(id);
        });
        layout->addWidget(button);

        //ui->notificationsList->layout()->addWidget(frame);
        //ui->noNotifications->setVisible(false);
        frame->setProperty("summaryLabel", QVariant::fromValue(sumLabel));
        frame->setProperty("bodyLabel", QVariant::fromValue(bodyLabel));

        notificationFrames.insert(id, frame);

        emit numNotificationsChanged(notificationFrames.count());
    }
}

void InfoPaneDropdown::removeNotification(int id) {
    if (notificationFrames.keys().contains(id)) {
        delete notificationFrames.value(id);
        notificationFrames.remove(id);
    }

    emit numNotificationsChanged(notificationFrames.count());

    if (notificationFrames.count() == 0) {
        //ui->noNotifications->setVisible(true);
    }
}


void InfoPaneDropdown::on_redshiftPause_toggled(bool checked)
{
    processTimer();
    settings.setValue("display/redshiftPaused", !checked);
}

void InfoPaneDropdown::updateSysInfo() {
    ui->currentBattery->setText(tr("Current Battery Percentage: %1").arg(QString::number(updbus->currentBattery()).append("%")));

    QTime uptime(0, 0);
    uptime = uptime.addMSecs(startTime.elapsed());
    ui->theshellUptime->setText(tr("theShell Uptime: %1").arg(uptime.toString("hh:mm:ss")));

    struct sysinfo* info = new struct sysinfo;
    if (sysinfo(info) == 0) {
        QTime sysUptime(0, 0);
        sysUptime = sysUptime.addSecs(info->uptime);
        QString uptimeString;
        if (info->uptime > 86400) {
            int days = info->uptime / 86400;
            uptimeString = tr("%n days", NULL, days) + " " + sysUptime.toString("hh:mm:ss");
        } else {
            uptimeString = sysUptime.toString("hh:mm:ss");
        }
        ui->systemUptime->setText(tr("System Uptime: %1").arg(uptimeString));
    } else {
        ui->systemUptime->setText(tr("Couldn't get system uptime"));
    }
    delete info;
}

void InfoPaneDropdown::on_resetButton_clicked()
{
    if (QMessageBox::warning(this, tr("Reset theShell"),
                             tr("All settings will be reset to default, and you will be logged out. "
                             "Are you sure you want to do this?"), QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) == QMessageBox::Yes) {
        settings.clear();
        EndSession(EndSessionWait::logout);
    }
}

void InfoPaneDropdown::mousePressEvent(QMouseEvent *event) {
    mouseClickPoint = event->localPos().toPoint().y();
    initialPoint = mouseClickPoint;
    dragRect = this->geometry();
    mouseMovedUp = false;
    draggingInfoPane = true;
    event->accept();
}

void InfoPaneDropdown::mouseMoveEvent(QMouseEvent *event) {
    if (draggingInfoPane) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();

        if (event->globalY() < mouseClickPoint) {
            mouseMovedUp = true;
        } else {
            mouseMovedUp = false;
        }

        //dragRect.translate(0, event->localPos().toPoint().y() - mouseClickPoint + this->y());
        dragRect = screenGeometry;
        dragRect.translate(0, event->globalY() - (initialPoint + screenGeometry.top()));

        //innerRect.translate(event->localPos().toPoint().y() - mouseClickPoint, 0);

        if (settings.value("bar/onTop", true).toBool()) {
            if (dragRect.bottom() >= screenGeometry.bottom()) {
                dragRect.moveTo(screenGeometry.left(), screenGeometry.top());
            }
        } else {
            if (dragRect.top() <= screenGeometry.top() - 1) {
                dragRect.moveTo(screenGeometry.left(), screenGeometry.top() - 1);
            }
        }
        this->setGeometry(dragRect);

        mouseClickPoint = event->globalY();
        event->accept();
    }
}

void InfoPaneDropdown::mouseReleaseEvent(QMouseEvent *event) {
    if (draggingInfoPane) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        if (initialPoint - 5 > mouseClickPoint && initialPoint + 5 < mouseClickPoint) {
            tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
            a->setStartValue(this->geometry());
            a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - (settings.value("bar/onTop", true).toBool() ? 0 : 1), this->width(), this->height()));
            a->setEasingCurve(QEasingCurve::OutCubic);
            a->setDuration(500);
            connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
            a->start();
        } else {
            /*if ((mouseMovedUp && settings.value("bar/onTop", true).toBool()) ||
                    (!mouseMovedUp && !settings.value("bar/onTop", true).toBool())) {*/
            if (mouseMovedUp == settings.value("bar/onTop", true).toBool()) {
                this->close();
            } else {
                tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
                a->setStartValue(this->geometry());
                a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - (settings.value("bar/onTop", true).toBool() ? 0 : 1), this->width(), this->height()));
                a->setEasingCurve(QEasingCurve::OutCubic);
                a->setDuration(500);
                connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
                a->start();
            }
        }
        event->accept();
        initialPoint = 0;
        draggingInfoPane = false;
    }
}

void InfoPaneDropdown::on_TouchFeedbackSwitch_toggled(bool checked)
{
    settings.setValue("input/touchFeedbackSound", checked);
}

void InfoPaneDropdown::on_brightnessSlider_sliderMoved(int position)
{
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -set " + QString::number(position));
    connect(backlight, SIGNAL(finished(int)), backlight, SLOT(deleteLater()));
}

void InfoPaneDropdown::on_brightnessSlider_valueChanged(int value)
{
    on_brightnessSlider_sliderMoved(value);
}

void InfoPaneDropdown::on_settingsList_currentRowChanged(int currentRow)
{
    ui->settingsTabs->setCurrentIndex(currentRow);

    //Set up settings
    if (ui->settingsTabs->currentWidget() == ui->NotificationsSettings) { //Notifications
        setupNotificationsSettingsPane();
    } else if (currentRow == 6) { //Location
        setupLocationSettingsPane();
    } else if (ui->settingsTabs->currentWidget() == ui->UserSettings) { //Users
        setupUsersSettingsPane();
    } else if (ui->settingsTabs->currentWidget() == ui->DateTimeSettings) { //Date and Time
        setupDateTimeSettingsPane();
    }
}

void InfoPaneDropdown::setupNotificationsSettingsPane() {
    ui->AppNotifications->clear();

    QStringList knownApplications;
    int amount = notificationAppSettings->beginReadArray("notifications/knownApplications");
    for (int i = 0; i < amount; i++) {
        notificationAppSettings->setArrayIndex(i);
        knownApplications.append(notificationAppSettings->value("appname").toString());
    }
    notificationAppSettings->endArray();

    for (QString app : knownApplications) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(app);
        ui->AppNotifications->addItem(item);
    }
}

void InfoPaneDropdown::on_settingsTabs_currentChanged(int arg1)
{
    ui->settingsList->item(arg1)->setSelected(true);
}

void InfoPaneDropdown::on_lockScreenBackgroundBrowse_clicked()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setNameFilter("Images (*.jpg *.jpeg *.bmp *.png *.gif *.svg)");
    if (dialog.exec() == QDialog::Accepted) {
        lockScreenSettings->setValue("background", dialog.selectedFiles().first());
        ui->lockScreenBackground->setText(dialog.selectedFiles().first());
    }
}

void InfoPaneDropdown::on_lockScreenBackground_textEdited(const QString &arg1)
{
    lockScreenSettings->setValue("background", arg1);
}

void InfoPaneDropdown::on_TextSwitch_toggled(bool checked)
{
    settings.setValue("bar/showText", checked);
}

void InfoPaneDropdown::on_windowManager_textEdited(const QString &arg1)
{
    settings.setValue("startup/WindowManagerCommand", arg1);
}

void InfoPaneDropdown::on_barDesktopsSwitch_toggled(bool checked)
{
    settings.setValue("bar/showWindowsFromOtherDesktops", checked);
}

void InfoPaneDropdown::on_BluetoothSwitch_toggled(bool checked)
{
    QDBusInterface("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", QDBusConnection::sessionBus()).setProperty("BluetoothEnabled", checked);
}

void InfoPaneDropdown::on_SuperkeyGatewaySwitch_toggled(bool checked)
{
    settings.setValue("input/superkeyGateway", checked);
}

void InfoPaneDropdown::reject() {
    this->close();
}

void InfoPaneDropdown::on_kdeconnectLabel_clicked()
{
    changeDropDown(KDEConnect);
}

void InfoPaneDropdown::on_endSessionConfirmFullScreen_toggled(bool checked)
{
    if (checked) {
        settings.setValue("ui/useFullScreenEndSession", true);
    }
}

void InfoPaneDropdown::on_endSessionConfirmInMenu_toggled(bool checked)
{
    if (checked) {
        settings.setValue("ui/useFullScreenEndSession", false);
    }
}

void InfoPaneDropdown::on_pageStack_switchingFrame(int switchTo)
{
    QWidget* switchingWidget = ui->pageStack->widget(switchTo);
    ui->clockLabel->setShowDisabled(true);
    ui->batteryLabel->setShowDisabled(true);
    ui->notificationsLabel->setShowDisabled(true);
    ui->networkLabel->setShowDisabled(true);
    //ui->printLabel->setShowDisabled(true);
    ui->kdeconnectLabel->setShowDisabled(true);

    if (switchingWidget == overviewFrame) {
        ui->clockLabel->setShowDisabled(false);
    } else if (switchingWidget == ui->statusFrame) {
        ui->batteryLabel->setShowDisabled(false);
    } else if (switchingWidget == ui->notificationsFrame) {
        ui->notificationsLabel->setShowDisabled(false);
    } else if (switchingWidget == ui->networkFrame) {
        ui->networkLabel->setShowDisabled(false);
    /*} else if (switchingWidget == ui->printFrame) {
        ui->printLabel->setShowDisabled(false);*/
    } else if (switchingWidget == ui->kdeConnectFrame) {
        ui->kdeconnectLabel->setShowDisabled(false);
    }
}

void InfoPaneDropdown::on_showNotificationsContents_toggled(bool checked)
{
    if (checked) {
        settings.setValue("notifications/lockScreen", "contents");
    }
}

void InfoPaneDropdown::on_showNotificationsOnly_toggled(bool checked)
{
    if (checked) {
        settings.setValue("notifications/lockScreen", "noContents");
    }
}

void InfoPaneDropdown::on_showNotificationsNo_toggled(bool checked)
{
    if (checked) {
        settings.setValue("notifications/lockScreen", "none");
    }
}

void InfoPaneDropdown::on_MediaSwitch_toggled(bool checked)
{
    settings.setValue("notifications/mediaInsert", checked);
}

void InfoPaneDropdown::on_lightColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "light");
        updateAccentColourBox();
        resetStyle();
        changeDropDown(Settings, false);
    }
}

void InfoPaneDropdown::on_darkColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "dark");
        updateAccentColourBox();
        resetStyle();
        changeDropDown(Settings, false);
    }
}

void InfoPaneDropdown::on_themeButtonColor_currentIndexChanged(int index)
{
    themeSettings->setValue("color/accent", index);
    resetStyle();
}

void InfoPaneDropdown::on_systemFont_currentFontChanged(const QFont &f)
{
    themeSettings->setValue("fonts/defaultFamily", f.family());
    themeSettings->setValue("fonts/smallFamily", f.family());
    ui->systemFontSize->setValue(themeSettings->value("font/defaultSize", 10).toInt());
    //ui->systemFont->setFont(QFont(themeSettings->value("font/defaultFamily", defaultFont).toString(), themeSettings->value("font/defaultSize", 10).toInt()));
}

void InfoPaneDropdown::on_batteryChartUpdateButton_clicked()
{
    updateBatteryChart();
}

//DBus Battery Info Structure
struct BatteryInfo {
    uint time, state;
    double value;
};
Q_DECLARE_METATYPE(BatteryInfo)

const QDBusArgument &operator<<(QDBusArgument &argument, const BatteryInfo &info) {
    argument.beginStructure();
    argument << info.time << info.value << info.state;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, BatteryInfo &info) {
    argument.beginStructure();
    argument >> info.time >> info.value >> info.state;
    argument.endStructure();
    return argument;
}

//DBus WakeupsInfo Structure
struct WakeupsInfo {
    bool process = false;
    uint pid;
    double wakeups;
    QString path, description;
};
Q_DECLARE_METATYPE(WakeupsInfo)

const QDBusArgument &operator<<(QDBusArgument &argument, const WakeupsInfo &info) {
    argument.beginStructure();
    argument << info.process << info.pid << info.wakeups << info.path << info.description;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WakeupsInfo &info) {
    argument.beginStructure();
    argument >> info.process >> info.pid >> info.wakeups >> info.path >> info.description;
    argument.endStructure();
    return argument;
}

void InfoPaneDropdown::updateBatteryChart() {
    if (ui->appsGraphButton->isChecked()) {
        QDBusMessage dataMessage = QDBusMessage::createMethodCall("org.freedesktop.UPower", "/org/freedesktop/UPower/Wakeups", "org.freedesktop.UPower.Wakeups", "GetData");

        QDBusReply<QDBusArgument> dataMessageArgument = QDBusConnection::systemBus().call(dataMessage);
        QList<WakeupsInfo> wakeups;

        if (dataMessageArgument.isValid()) {
            QDBusArgument arrayArgument = dataMessageArgument.value();
            arrayArgument.beginArray();
            while (!arrayArgument.atEnd()) {
                WakeupsInfo info;
                arrayArgument >> info;

                if (info.process) {
                    int min = 0, max = wakeups.count();
                    int insertIndex;

                    while (max != min) {
                        insertIndex = ((max - min) / 2) + min;
                        if (wakeups.at(insertIndex).wakeups == info.wakeups) { //Goes here
                            break;
                        } else if (wakeups.at(insertIndex).wakeups < info.wakeups) { //Needs to go on left hand side
                            max = insertIndex - 1;
                        } else if (wakeups.at(insertIndex).wakeups > info.wakeups) { //Needs to go on right hand side
                            min = insertIndex + 1;
                        }
                    }

                    wakeups.insert(insertIndex, info);
                }
            }
            arrayArgument.endArray();

            ui->appsGraph->clear();
            for (WakeupsInfo wakeup : wakeups) {
                QListWidgetItem* item = new QListWidgetItem;
                item->setText("[" + QString::number(wakeup.pid) + "] " + wakeup.path + " (" + wakeup.description + ")");
                ui->appsGraph->insertItem(0, item);
            }
        }

    } else {
        for (QAbstractAxis* axis : batteryChart->axes()) {
            batteryChart->removeAxis(axis);
            axis->deleteLater();
        }

        QDBusMessage historyMessage = QDBusMessage::createMethodCall("org.freedesktop.UPower", updbus->defaultBattery().path(), "org.freedesktop.UPower.Device", "GetHistory");
        QVariantList historyMessageArguments;

        if (ui->chargeGraphButton->isChecked()) {
            historyMessageArguments.append("charge");
        } else {
            historyMessageArguments.append("rate");
        }

        historyMessageArguments.append((uint) 0); //Get surplus data so we can plot some data off the left of the graph
        historyMessageArguments.append((uint) 10000);
        historyMessage.setArguments(historyMessageArguments);

        QDBusReply<QDBusArgument> historyArgument = QDBusConnection::systemBus().call(historyMessage);

        QLineSeries* batteryChartData = new QLineSeries;
        QPen dataPen;
        dataPen.setColor(this->palette().color(QPalette::Highlight));
        dataPen.setWidth(2 * getDPIScaling());
        batteryChartData->setPen(dataPen);

        QLineSeries* batteryChartTimeRemainingData = new QLineSeries;
        //batteryChartTimeRemainingData->setColor(this->palette().color(QPalette::Disabled, QPalette::WindowText));
        batteryChartTimeRemainingData->setBrush(QBrush(this->palette().color(QPalette::Disabled, QPalette::WindowText)));

        QPen remainingTimePen;
        remainingTimePen.setColor(this->palette().color(QPalette::Disabled, QPalette::Highlight));
        remainingTimePen.setDashPattern(QVector<qreal>() << 3 << 3);
        remainingTimePen.setDashOffset(3);
        remainingTimePen.setWidth(2 * getDPIScaling());
        batteryChartTimeRemainingData->setPen(remainingTimePen);

        QDateTime remainingTime = updbus->batteryTimeRemaining();

        int firstDateTime = QDateTime::currentSecsSinceEpoch() / 60;
        qint64 msecsSinceFull = -1;
        uint lastState = -1;
        bool takeNextSinceLastFull = false;
        if (historyArgument.isValid()) {
            QDBusArgument arrayArgument = historyArgument.value();
            arrayArgument.beginArray();
            while (!arrayArgument.atEnd()) {
                BatteryInfo info;
                arrayArgument >> info;

                qint64 msecs = info.time;
                msecs = msecs * 1000;

                if (info.value >= 90 && info.state == 2 && lastState == 1 && msecsSinceFull < msecs) {
                    takeNextSinceLastFull = true;
                } else if (takeNextSinceLastFull) {
                    takeNextSinceLastFull = false;
                    msecsSinceFull = msecs;
                }
                lastState = info.state;

                if (info.value != 0 && info.state != 0) {
                    batteryChartData->append(msecs, info.value);
                    if (firstDateTime > info.time / 60) {
                        firstDateTime = info.time / 60;
                    }
                }
            }
            arrayArgument.endArray();
            batteryChartData->append(QDateTime::currentMSecsSinceEpoch(), batteryChartData->at(batteryChartData->count() - 1).y());

            if (remainingTime.isValid() && ui->batteryChartShowProjected->isChecked() && ui->chargeGraphButton->isChecked()) {
                QDateTime lastDateTime = QDateTime::fromMSecsSinceEpoch(batteryChartData->at(batteryChartData->count() - 1).x());
                batteryChartTimeRemainingData->append(batteryChartData->at(batteryChartData->count() - 1));
                QDateTime endDateTime = lastDateTime.addMSecs(remainingTime.toMSecsSinceEpoch());
                if (updbus->charging()) {
                    batteryChartTimeRemainingData->append(endDateTime.toMSecsSinceEpoch(), 100);
                } else {
                    batteryChartTimeRemainingData->append(endDateTime.toMSecsSinceEpoch(), 0);
                }
            }
        }

        batteryChart->removeAllSeries();
        batteryChart->addSeries(batteryChartData);
        batteryChart->addSeries(batteryChartTimeRemainingData);

        xAxis = new QDateTimeAxis;
        if (ui->chargeGraphButton->isChecked()) {
            if (remainingTime.isValid() && ui->batteryChartShowProjected->isChecked()) {
                xAxis->setMax(QDateTime::fromMSecsSinceEpoch(batteryChartData->at(batteryChartData->count() - 1).x()).addMSecs(remainingTime.toMSecsSinceEpoch()));
            } else {
                xAxis->setMax(QDateTime::currentDateTime());
            }

            QDateTime oneDay = xAxis->max().addDays(-1);
            if (msecsSinceFull == -1 || msecsSinceFull < oneDay.toMSecsSinceEpoch()) {
                xAxis->setMin(oneDay);
            } else {
                xAxis->setMin(QDateTime::fromMSecsSinceEpoch(msecsSinceFull));
            }
        } else {
            xAxis->setMax(QDateTime::currentDateTime());
            xAxis->setMin(xAxis->max().addSecs(-43200)); //Half a day
        }
        batteryChart->addAxis(xAxis, Qt::AlignBottom);
        xAxis->setLabelsColor(this->palette().color(QPalette::WindowText));
        xAxis->setFormat("hh:mm");
        xAxis->setTickCount(9);
        batteryChartData->attachAxis(xAxis);
        batteryChartTimeRemainingData->attachAxis(xAxis);

        /*connect(xAxis, &QDateTimeAxis::rangeChanged, [=](QDateTime min, QDateTime max) {
            ui->BatteryChargeScrollBar->setMaximum(max.toMSecsSinceEpoch() - min.toMSecsSinceEpoch());
        });*/

        chartScrolling = true;
        int currentSecsSinceEpoch = QDateTime::currentSecsSinceEpoch();
        ui->BatteryChargeScrollBar->setMinimum(0);
        ui->BatteryChargeScrollBar->setMaximum(currentSecsSinceEpoch / 60 - firstDateTime);
        ui->BatteryChargeScrollBar->setValue(currentSecsSinceEpoch / 60 - firstDateTime);
        startValue = currentSecsSinceEpoch / 60 - firstDateTime;
        chartScrolling = false;

        QValueAxis* yAxis = new QValueAxis;
        if (ui->chargeGraphButton->isChecked()) {
            yAxis->setLabelFormat("%i%%");
            yAxis->setMax(100);
        } else {
            yAxis->setLabelFormat("%i W");
            yAxis->setMax(40);
        }

        yAxis->setMin(0);
        yAxis->setLabelsColor(this->palette().color(QPalette::WindowText));
        batteryChart->addAxis(yAxis, Qt::AlignLeft);
        batteryChartData->attachAxis(yAxis);
        batteryChartTimeRemainingData->attachAxis(yAxis);

        ui->batteryChartLastUpdate->setText(tr("Last updated %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    }
}

void InfoPaneDropdown::on_batteryChartShowProjected_toggled(bool checked)
{
    Q_UNUSED(checked)
    updateBatteryChart();
}

void InfoPaneDropdown::on_upArrow_clicked()
{
    this->close();
}

void InfoPaneDropdown::on_PowerStretchSwitch_toggled(bool checked)
{
    updbus->setPowerStretch(checked);
    emit batteryStretchChanged(checked);
}

void InfoPaneDropdown::doNetworkCheck() {
    if (updbus->powerStretch()) {
        //Always set networkOk to ok because we don't update when power stretch is on
        networkOk = Ok;
    } else {
        //Do some network checks to see if network is working

        QDBusInterface i("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
        int connectivity = i.property("Connectivity").toUInt();
        if (connectivity == 2) {
            if (networkOk != BehindPortal) {
                //Notify user that they are behind a portal.
                //Wait 10 seconds for startup or for connection notification

                QTimer::singleShot(10000, [=] {
                    QStringList actions;
                    actions.append("login");
                    actions.append(tr("Log in to network"));

                    QVariantMap hints;
                    hints.insert("category", "network.connected");
                    hints.insert("transient", true);

                    uint notificationId = ndbus->Notify("theShell", 0, "", tr("Network Login"),
                                               tr("Your connection to the internet is blocked by a login page."),
                                               actions, hints, 30000);
                    connect(ndbus, &NotificationsDBusAdaptor::ActionInvoked, [=](uint id, QString key) {
                        if (notificationId == id && key == "login") {
                            QProcess::startDetached("xdg-open http://nmcheck.gnome.org/");
                        }
                    });
                });
            }

            networkOk = BehindPortal;

            //Reload the connectivity status
            i.asyncCall("CheckConnectivity");
            return;
        } else if (connectivity == 3) {
            networkOk = Unspecified;

            //Reload the connectivity status
            i.asyncCall("CheckConnectivity");
            return;
        } else {
            networkOk = Ok;
        }

        QNetworkAccessManager* manager = new QNetworkAccessManager;
        if (manager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
            networkOk = Unspecified;
            manager->deleteLater();

            //Reload the connectivity status
            i.asyncCall("CheckConnectivity");
            return;
        }
        manager->deleteLater();

        //For some reason this crashes theShell so let's not do this (for now)
        /*connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply* reply) {
            if (reply->error() != QNetworkReply::NoError) {
                networkOk = false;
            } else {
                networkOk = true;
            }
            manager->deleteLater();
        });
        manager->get(QNetworkRequest(QUrl("http://vicr123.github.io/")));*/

        //Reload the connectivity status
        i.asyncCall("CheckConnectivity");
    }
}

void InfoPaneDropdown::dragDown(dropdownType showWith, int y) {
    changeDropDown(showWith, false);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    if (settings.value("bar/onTop", true).toBool()) {
        this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height() + y, screenGeometry.width(), screenGeometry.height() + 1);
    } else {
        this->setGeometry(screenGeometry.x(), screenGeometry.top() + y + screenGeometry.y(), screenGeometry.width(), screenGeometry.height() + 1);
    }

    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False);
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();

    this->setFixedWidth(screenGeometry.width());

    if (tVirtualKeyboard::instance()->keyboardVisible()) {
        this->setFixedHeight(screenGeometry.height() - tVirtualKeyboard::instance()->height());
    } else {
        this->setFixedHeight(screenGeometry.height() + 1);
    }

    //Get Current Brightness
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -get");
    backlight->waitForFinished();
    float output = ceil(QString(backlight->readAll()).toFloat());
    delete backlight;

    ui->brightnessSlider->setValue((int) output);

    previousDragY = y;
}

void InfoPaneDropdown::completeDragDown() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    if ((QCursor::pos().y() - screenGeometry.top() < previousDragY && settings.value("bar/onTop", true).toBool()) ||
            (QCursor::pos().y() - screenGeometry.top() > previousDragY && !settings.value("bar/onTop", true).toBool())) {
        this->close();
    } else {
        tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
        a->setStartValue(this->geometry());
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - (settings.value("bar/onTop", true).toBool() ? 0 : 1), this->width(), screenGeometry.height() + 1));
        a->setEasingCurve(QEasingCurve::OutCubic);
        a->setDuration(500);
        connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
        a->start();
    }
}

void InfoPaneDropdown::on_notificationSoundBox_currentIndexChanged(int index)
{
    QSoundEffect* sound = new QSoundEffect();
    switch (index) {
        case 0:
            settings.setValue("notifications/sound", "tripleping");
            sound->setSource(QUrl("qrc:/sounds/notifications/tripleping.wav"));
            break;
        case 1:
            settings.setValue("notifications/sound", "upsidedown");
            sound->setSource(QUrl("qrc:/sounds/notifications/upsidedown.wav"));
            break;
        case 2:
            settings.setValue("notifications/sound", "echo");
            sound->setSource(QUrl("qrc:/sounds/notifications/echo.wav"));
            break;
    }
    sound->play();
    connect(sound, SIGNAL(playingChanged()), sound, SLOT(deleteLater()));
}

void InfoPaneDropdown::setupUsersSettingsPane() {
    ui->availableUsersWidget->clear();

    QDBusMessage getUsersMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "ListCachedUsers");
    QDBusReply<QList<QDBusObjectPath>> allUsers = QDBusConnection::systemBus().call(getUsersMessage);
    if (allUsers.isValid()) {
        for (QDBusObjectPath obj : allUsers.value()) {
            QDBusInterface interface("org.freedesktop.Accounts", obj.path(), "org.freedesktop.Accounts.User", QDBusConnection::systemBus());

            QListWidgetItem* item = new QListWidgetItem();
            QString name = interface.property("RealName").toString();
            if (name == "") {
                name = interface.property("UserName").toString();
            }
            item->setText(name);
            item->setIcon(QIcon::fromTheme("user"));
            item->setData(Qt::UserRole, obj.path());
            ui->availableUsersWidget->addItem(item);
        }

        QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(QIcon::fromTheme("list-add"));
        item->setText(tr("Add New User"));
        item->setData(Qt::UserRole, "new");
        ui->availableUsersWidget->addItem(item);
    }
}

void InfoPaneDropdown::on_userSettingsNextButton_clicked()
{
    if (ui->availableUsersWidget->selectedItems().count() != 0) {
        //Check Polkit authorization
        PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.freedesktop.accounts.user-administration", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);
        if (r == PolkitQt1::Authority::No) {
            QMessageBox::warning(this, tr("Unauthorized"), tr("Polkit does not allow you to manage users on the system."), QMessageBox::Ok, QMessageBox::Ok);
            return;
        } else if (r == PolkitQt1::Authority::Challenge) {
            LOWER_INFOPANE
            PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.freedesktop.accounts.user-administration", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::AllowUserInteraction);
            if (r != PolkitQt1::Authority::Yes) {
                return;
            }
        }

        editingUserPath = ui->availableUsersWidget->selectedItems().first()->data(Qt::UserRole).toString();
        if (editingUserPath == "new") {
            ui->userSettingsEditUserLabel->setText(tr("New User"));
            ui->userSettingsFullName->setText("");
            ui->userSettingsUserName->setText("");
            ui->userSettingsPassword->setPlaceholderText(tr("(none)"));
            ui->userSettingsPasswordCheck->setPlaceholderText(tr("(none)"));
            ui->userSettingsDeleteUser->setVisible(false);
            ui->userSettingsStandardAccount->setChecked(true);
            ui->userSettingsAdminAccount->setChecked(false);
        } else {
            ui->userSettingsEditUserLabel->setText(tr("Edit User"));
            QDBusInterface interface("org.freedesktop.Accounts", editingUserPath, "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
            int passwordMode = interface.property("PasswordMode").toInt();
            if (passwordMode == 0) {
                ui->userSettingsPassword->setPlaceholderText(tr("(unchanged)"));
                ui->userSettingsPasswordCheck->setPlaceholderText(tr("(unchanged)"));
            } else if (passwordMode == 1) {
                ui->userSettingsPassword->setPlaceholderText(tr("(set at next login)"));
                ui->userSettingsPasswordCheck->setPlaceholderText(tr("(set at next login)"));
            } else {
                ui->userSettingsPassword->setPlaceholderText(tr("(none)"));
                ui->userSettingsPasswordCheck->setPlaceholderText(tr("(none)"));
            }
            if (interface.property("AccountType").toInt() == 0) {
                ui->userSettingsStandardAccount->setChecked(true);
                ui->userSettingsAdminAccount->setChecked(false);
            } else {
                ui->userSettingsStandardAccount->setChecked(false);
                ui->userSettingsAdminAccount->setChecked(true);
            }
            ui->userSettingsFullName->setText(interface.property("RealName").toString());
            ui->userSettingsUserName->setText(interface.property("UserName").toString());
            ui->userSettingsPasswordHint->setText(interface.property("PasswordHint").toString());
            ui->userSettingsDeleteUser->setVisible(true);
        }
        ui->userSettingsPassword->setText("");
        ui->userSettingsPasswordCheck->setText("");
        ui->userSettingsStackedWidget->setCurrentIndex(1);
    }
}

void InfoPaneDropdown::on_userSettingsCancelButton_clicked()
{
    ui->userSettingsStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_userSettingsApplyButton_clicked()
{
    if (ui->userSettingsPasswordCheck->text() != ui->userSettingsPassword->text()) {
        QMessageBox::warning(this, tr("Password Check"), tr("The passwords don't match."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (ui->userSettingsUserName->text().contains(" ")) {
        QMessageBox::warning(this, tr("Username"), tr("The username must not contain spaces."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (ui->userSettingsUserName->text().toLower() != ui->userSettingsUserName->text()) {
        QMessageBox::warning(this, tr("Username"), tr("The username must not contain capital letters."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    if (editingUserPath == "new") {
        QDBusMessage createMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "CreateUser");
        QVariantList args;
        args.append(ui->userSettingsUserName->text());
        args.append(ui->userSettingsFullName->text());
        args.append(0);
        createMessage.setArguments(args);

        QDBusReply<QDBusObjectPath> newUser = QDBusConnection::systemBus().call(createMessage);
        if (newUser.error().isValid()) {
            tToast* toast = new tToast();
            toast->setTitle("Couldn't create user");
            toast->setText(newUser.error().message());
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this);
            return;
        } else {
            editingUserPath = newUser.value().path();
        }
    }

    QDBusInterface interface("org.freedesktop.Accounts", editingUserPath, "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
    QDBusMessage setUserNameMessage = interface.call("SetUserName", ui->userSettingsUserName->text());
    if (setUserNameMessage.errorMessage() != "") {
        tToast* toast = new tToast();
        toast->setTitle("Couldn't create user");
        toast->setText(setUserNameMessage.errorMessage());
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
        toast->show(this);
        return;
    }
    interface.call("SetRealName", ui->userSettingsFullName->text());

    if (ui->userSettingsAdminAccount->isChecked()) {
        interface.call("SetAccountType", 1);
    } else {
        interface.call("SetAccountType", 0);
    }

    if (ui->userSettingsPassword->text() != "") {
        interface.call("SetPasswordMode", 0);

        //Crypt password
        QByteArray characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvxyz./";
        QByteArray salt("$6$");
        for (int i = 0; i < 16; i++) {
            salt.append(characters.at((qrand() % characters.count())));
        }
        QString cryptedPassword = QString::fromLatin1(crypt(ui->userSettingsPassword->text().toUtf8(), salt.constData()));

        interface.call("SetPassword", cryptedPassword, ui->userSettingsPasswordHint->text());
    } else {
        if (editingUserPath == "new") {
            interface.call("SetPasswordMode", 2);
            interface.call("SetPasswordHint", ui->userSettingsPasswordHint->text());
        }
    }

    setupUsersSettingsPane();
    ui->userSettingsStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_userSettingsFullName_textEdited(const QString &arg1)
{
    ui->userSettingsUserName->setText(arg1.toLower().split(" ").first());
}

void InfoPaneDropdown::on_userSettingsDeleteUser_clicked()
{
    ui->userSettingsStackedWidget->setCurrentIndex(2);
}

void InfoPaneDropdown::on_userSettingsCancelDeleteUser_clicked()
{
    ui->userSettingsStackedWidget->setCurrentIndex(1);
}

void InfoPaneDropdown::on_userSettingsDeleteUserOnly_clicked()
{
    QDBusInterface interface("org.freedesktop.Accounts", editingUserPath, "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
    qlonglong uid = interface.property("Uid").toLongLong();

    QDBusMessage deleteMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "DeleteUser");
    QVariantList args;
    args.append(uid);
    args.append(false);
    deleteMessage.setArguments(args);
    QDBusConnection::systemBus().call(deleteMessage);

    setupUsersSettingsPane();
    ui->userSettingsStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_userSettingsDeleteUserAndData_clicked()
{
    QDBusInterface interface("org.freedesktop.Accounts", editingUserPath, "org.freedesktop.Accounts.User", QDBusConnection::systemBus());
    qlonglong uid = interface.property("Uid").toLongLong();

    QDBusMessage deleteMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "DeleteUser");
    QVariantList args;
    args.append(uid);
    args.append(true);
    deleteMessage.setArguments(args);
    QDBusConnection::systemBus().call(deleteMessage);

    setupUsersSettingsPane();
    ui->userSettingsStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::setupDateTimeSettingsPane() {
    launchDateTimeService();

    QDateTime current = QDateTime::currentDateTime();
    ui->dateTimeSetDate->setSelectedDate(current.date());
    ui->dateTimeSetTime->setTime(current.time());

    QDBusInterface dateTimeInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    bool isNTPEnabled = dateTimeInterface.property("NTP").toBool();
    ui->DateTimeNTPSwitch->setChecked(isNTPEnabled);
}

void InfoPaneDropdown::launchDateTimeService() {
    QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
    if (!reply.value().contains("org.freedesktop.timedate1")) {
        qDebug() << "Can't set date and time";
        return;
    }

    QDBusConnection::systemBus().interface()->startService("org.freedesktop.timedate1");
}

void InfoPaneDropdown::on_dateTimeSetDateTimeButton_clicked()
{
    QDateTime newTime;
    newTime.setDate(ui->dateTimeSetDate->selectedDate());
    newTime.setTime(ui->dateTimeSetTime->time());

    qlonglong time = newTime.toMSecsSinceEpoch() * 1000;

    launchDateTimeService();

    QDBusMessage setMessage = QDBusMessage::createMethodCall("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", "SetTime");
    QVariantList args;
    args.append(time);
    args.append(false);
    args.append(true);
    setMessage.setArguments(args);
    QDBusConnection::systemBus().call(setMessage);

    setupDateTimeSettingsPane();
}

void InfoPaneDropdown::on_DateTimeNTPSwitch_toggled(bool checked)
{
    if (checked) {
        ui->dateTimeSetDate->setEnabled(false);
        ui->dateTimeSetTime->setEnabled(false);
        ui->dateTimeSetDateTimeButton->setEnabled(false);
    } else {
        ui->dateTimeSetDate->setEnabled(true);
        ui->dateTimeSetTime->setEnabled(true);
        ui->dateTimeSetDateTimeButton->setEnabled(true);
    }

    launchDateTimeService();

    QDBusMessage setMessage = QDBusMessage::createMethodCall("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", "SetNTP");
    QVariantList args;
    args.append(checked);
    args.append(true);
    setMessage.setArguments(args);
    QDBusConnection::systemBus().call(setMessage);

    setupDateTimeSettingsPane();
}

void InfoPaneDropdown::on_localeList_currentRowChanged(int currentRow)
{
    //Show the splash screen (if available)
    emit dbusSignals->ShowSplash();

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
        case Internationalisation::idID:
            settings.setValue("locale/language", "id_ID");
            break;
        case Internationalisation::auAU:
            settings.setValue("locale/language", "au_AU");
            break;
        case Internationalisation::nbNO:
            settings.setValue("locale/language", "nb_NO");
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

    if (defaultLocale.name() == "C") {
        tsTranslator->load(localeName, QString(SHAREDIR) + "translations");
    } else {
        tsTranslator->load(defaultLocale.name(), QString(SHAREDIR) + "translations");
    }
    QApplication::installTranslator(tsTranslator);

    //Tell all plugins to update translator
    for (StatusCenterPane* plugin : loadedPlugins) {
        plugin->loadLanguage(defaultLocale.name());
    }

    //Fill locale box
    Internationalisation::fillLanguageBox(ui->localeList);

    //Process all events
    QApplication::processEvents();

    //Hide the splash screen since the language change is complete
    emit dbusSignals->HideSplash();
}

void InfoPaneDropdown::on_StatusBarSwitch_toggled(bool checked)
{
    settings.setValue("bar/statusBar", checked);
    updateStruts();

    ui->AutoShowBarLabel->setEnabled(checked);
    ui->AutoShowBarSwitch->setEnabled(checked);
    ui->AutoShowBarExplanation->setEnabled(checked);
}

void InfoPaneDropdown::on_TouchInputSwitch_toggled(bool checked)
{
    settings.setValue("input/touch", checked);
}

void InfoPaneDropdown::on_quietModeSound_clicked()
{
    AudioMan->setQuietMode(AudioManager::none);
    ui->quietModeSound->setChecked(true);
}

void InfoPaneDropdown::on_quietModeNotification_clicked()
{
    AudioMan->setQuietMode(AudioManager::notifications);
    ui->quietModeNotification->setChecked(true);
}

void InfoPaneDropdown::on_quietModeMute_clicked()
{
    AudioMan->setQuietMode(AudioManager::mute);
    ui->quietModeMute->setChecked(true);
}

void InfoPaneDropdown::on_SuspendLockScreen_toggled(bool checked)
{
    settings.setValue("lockScreen/showOnSuspend", checked);
}

void InfoPaneDropdown::on_BatteryChargeScrollBar_valueChanged(int value)
{
    if (!chartScrolling) {
        chartScrolling = true;
        batteryChart->scroll(value - startValue, 0);
        startValue = value;
        chartScrolling = false;
    }
}

void InfoPaneDropdown::on_chargeGraphButton_clicked()
{
    ui->chargeGraphButton->setChecked(true);
    ui->rateGraphButton->setChecked(false);
    ui->appsGraphButton->setChecked(false);
    ui->batteryGraphStack->setCurrentIndex(0);
    ui->batteryChartHeader->setText(tr("Charge History"));
    ui->batteryChartShowProjected->setVisible(true);
    updateBatteryChart();
}

void InfoPaneDropdown::on_rateGraphButton_clicked()
{
    ui->chargeGraphButton->setChecked(false);
    ui->rateGraphButton->setChecked(true);
    ui->appsGraphButton->setChecked(false);
    ui->batteryGraphStack->setCurrentIndex(0);
    ui->batteryChartHeader->setText(tr("Rate History"));
    ui->batteryChartShowProjected->setVisible(false);
    updateBatteryChart();
}

void InfoPaneDropdown::on_appsGraphButton_clicked()
{
    ui->chargeGraphButton->setChecked(false);
    ui->rateGraphButton->setChecked(false);
    ui->appsGraphButton->setChecked(true);
    ui->batteryGraphStack->setCurrentIndex(1);
    ui->batteryChartHeader->setText(tr("Application Power Usage"));
    ui->batteryChartShowProjected->setVisible(false);
    updateBatteryChart();
}

void InfoPaneDropdown::on_LargeTextSwitch_toggled(bool checked)
{
    themeSettings->setValue("accessibility/largeText", checked);
}

void InfoPaneDropdown::on_HighContrastSwitch_toggled(bool checked)
{
    themeSettings->setValue("accessibility/highcontrast", checked);
    setHeaderColour(QColor(0, 100, 255));
}

void InfoPaneDropdown::on_systemAnimationsAccessibilitySwitch_toggled(bool checked)
{
    themeSettings->setValue("accessibility/systemAnimations", checked);
}

void InfoPaneDropdown::on_CapsNumLockBellSwitch_toggled(bool checked)
{
    themeSettings->setValue("accessibility/bellOnCapsNumLock", checked);
}

void InfoPaneDropdown::on_FlightSwitch_toggled(bool checked)
{
    QDBusInterface i("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);

    //Set flags that persist between changes
    settings.setValue("flightmode/on", checked);
    if (checked) {
        settings.setValue("flightmode/wifi", ui->WifiSwitch->isChecked());
        settings.setValue("flightmode/bt", ui->BluetoothSwitch->isChecked());

        //Disable bluetooth and WiFi.
        ui->WifiSwitch->setChecked(false);
        ui->BluetoothSwitch->setChecked(false);

        //Disable WiMAX and mobile networking
        i.setProperty("WwanEnabled", false);
        i.setProperty("WimaxEnabled", false);
    } else {
        //Enable bluetooth and WiFi.
        ui->WifiSwitch->setChecked(settings.value("flightmode/wifi", true).toBool());
        ui->BluetoothSwitch->setChecked(settings.value("flightmode/bt", true).toBool());

        //Enable WiMAX and mobile networking
        i.setProperty("WwanEnabled", true);
        i.setProperty("WimaxEnabled", true);
    }

    emit flightModeChanged(checked);

    //Don't disable the switch as they may be switched on during flight
}

void InfoPaneDropdown::on_TwentyFourHourSwitch_toggled(bool checked)
{
    settings.setValue("time/use24hour", checked);
}

void InfoPaneDropdown::on_systemIconTheme_currentIndexChanged(int index)
{
    themeSettings->setValue("icons/theme", ui->systemIconTheme->itemData(index).toString());
}

void InfoPaneDropdown::on_AttenuateSwitch_toggled(bool checked)
{
    settings.setValue("notifications/attenuate", checked);
}

void InfoPaneDropdown::on_BarOnBottom_toggled(bool checked)
{
    settings.setValue("bar/onTop", !checked);
    updateStruts();
}

void InfoPaneDropdown::updateStruts() {
    emit updateStrutsSignal();

    if (settings.value("bar/onTop", true).toBool()) {
        ((QBoxLayout*) this->layout())->setDirection(QBoxLayout::TopToBottom);
        ((QBoxLayout*) ui->partFrame->layout())->setDirection(QBoxLayout::TopToBottom);
        ((QBoxLayout*) ui->settingsFrame->layout())->setDirection(QBoxLayout::TopToBottom);
        ((QBoxLayout*) ui->kdeConnectFrame->layout())->setDirection(QBoxLayout::TopToBottom);
        ui->upArrow->setPixmap(QIcon::fromTheme("go-up").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    } else {
        ((QBoxLayout*) this->layout())->setDirection(QBoxLayout::BottomToTop);
        ((QBoxLayout*) ui->partFrame->layout())->setDirection(QBoxLayout::BottomToTop);
        ((QBoxLayout*) ui->settingsFrame->layout())->setDirection(QBoxLayout::BottomToTop);
        ((QBoxLayout*) ui->kdeConnectFrame->layout())->setDirection(QBoxLayout::BottomToTop);
        ui->upArrow->setPixmap(QIcon::fromTheme("go-down").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    }
}

void InfoPaneDropdown::on_systemWidgetTheme_currentIndexChanged(int index)
{
    themeSettings->setValue("style/name", ui->systemWidgetTheme->itemData(index).toString());
    resetStyle();
}

void InfoPaneDropdown::resetStyle() {
    emit dbusSignals->ThemeChanged();
}

void InfoPaneDropdown::on_decorativeColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "decorative");
        updateAccentColourBox();
        resetStyle();
    }
}

void InfoPaneDropdown::on_SoundFeedbackSoundSwitch_toggled(bool checked) {
    settings.setValue("sound/feedbackSound", checked);
}

void InfoPaneDropdown::on_VolumeOverdriveSwitch_toggled(bool checked) {
    settings.setValue("sound/volumeOverdrive", checked);
}

void InfoPaneDropdown::updateAccentColourBox() {
    //Set up theme button combo box
    int themeAccentColorIndex = themeSettings->value("color/accent", 0).toInt();

    ui->themeButtonColor->clear();
    if (themeSettings->value("color/type", "dark") == "decorative") {
        if (themeAccentColorIndex > 1) themeAccentColorIndex = 0;
        ui->themeButtonColor->addItem(tr("Oxygen"));
        ui->themeButtonColor->addItem(tr("Breeze"));
    } else {
        if (themeAccentColorIndex > 4) themeAccentColorIndex = 0;
        ui->themeButtonColor->addItem(tr("Blue"));
        ui->themeButtonColor->addItem(tr("Green"));
        ui->themeButtonColor->addItem(tr("Orange"));
        ui->themeButtonColor->addItem(tr("Pink"));
        ui->themeButtonColor->addItem(tr("Turquoise"));

        ui->themeButtonColor->setCurrentIndex(themeAccentColorIndex);
    }
}

void InfoPaneDropdown::on_dpi100_toggled(bool checked)
{
    if (checked) {
        settings.setValue("screen/dpi", 96);
    }
}

void InfoPaneDropdown::on_dpi150_toggled(bool checked)
{
    if (checked) {
        settings.setValue("screen/dpi", 144);
    }
}

void InfoPaneDropdown::on_dpi200_toggled(bool checked)
{
    if (checked) {
        settings.setValue("screen/dpi", 192);
    }
}

void InfoPaneDropdown::on_dpi300_toggled(bool checked)
{
    if (checked) {
        settings.setValue("screen/dpi", 288);
    }
}

void InfoPaneDropdown::on_AutoShowBarSwitch_toggled(bool checked)
{
    settings.setValue("bar/autoshow", checked);
}

void InfoPaneDropdown::on_userSettingsAdminAccount_toggled(bool checked)
{
    if (checked) {
        ui->userSettingsStandardAccount->setChecked(false);
        ui->userSettingsAdminAccount->setChecked(true);
    }
}

void InfoPaneDropdown::on_userSettingsStandardAccount_toggled(bool checked)
{
    if (checked) {
        ui->userSettingsStandardAccount->setChecked(true);
        ui->userSettingsAdminAccount->setChecked(false);
    }
}

void InfoPaneDropdown::updateAutostart() {
    ui->autostartList->clear();

    QDir autostartDir(QDir::homePath() + "/.config/autostart");
    for (QString fileName : autostartDir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        QString file = QDir::homePath() + "/.config/autostart/" + fileName;
        QFile autostartFile(file);
        autostartFile.open(QFile::ReadOnly);
        QString data = autostartFile.readAll();
        autostartFile.close();

        QString name = fileName;
        QString icon = "";
        bool enabled = true;
        bool validEntry = true;

        for (QString line : data.split("\n")) {
            QString data = line.mid(line.indexOf("=") + 1);
            if (line.startsWith("name=", Qt::CaseInsensitive)) {
                name = data;
            } else if (line.startsWith("onlyshowin=", Qt::CaseInsensitive)) {
                if (!data.contains("theshell", Qt::CaseInsensitive)) {
                    validEntry = false;
                }
            } else if (line.startsWith("notshowin=", Qt::CaseInsensitive)) {
                if (data.contains("theshell", Qt::CaseInsensitive)) {
                    validEntry = false;
                }
            } else if (line.startsWith("hidden=", Qt::CaseInsensitive)) {
                if (data.toLower() == "true") {
                    enabled = false;
                }
            } else if (line.startsWith("icon=")) {
                icon = data;
            }
        }

        if (validEntry) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            if (enabled) {
                item->setCheckState(Qt::Checked);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
            if (icon != "") {
                item->setIcon(QIcon::fromTheme(icon));
            }
            item->setText(name);
            item->setData(Qt::UserRole, file);

            ui->autostartList->addItem(item);
        }
    }
}
void InfoPaneDropdown::on_autostartList_itemChanged(QListWidgetItem *item)
{
    QFile file(item->data(Qt::UserRole).toString());

    file.open(QFile::ReadOnly);
    QString data = file.readAll();
    file.close();

    QString rewriteData;

    for (QString line : data.split("\n")) {
        if (!line.startsWith("hidden", Qt::CaseInsensitive)) {
            rewriteData.append(line + "\n");
        }
    }

    if (item->checkState() == Qt::Unchecked) {
        rewriteData.append("Hidden=true\n");
    }

    file.open(QFile::WriteOnly);
    file.write(rewriteData.toUtf8());
    file.close();

    this->updateAutostart();
}

void InfoPaneDropdown::on_backAutoStartApps_clicked()
{
    ui->startupStack->setCurrentIndex(0);
}

void InfoPaneDropdown::on_pushButton_4_clicked()
{
    ui->startupStack->setCurrentIndex(1);

    AppsListModel* appsListModel = new AppsListModel();
    ui->autostartAppList->setModel(appsListModel);
    ui->autostartAppList->setItemDelegate(new AppsDelegate(nullptr, false));
}

void InfoPaneDropdown::on_backAutoStartNewApp_clicked()
{
    ui->startupStack->setCurrentIndex(1);
}

void InfoPaneDropdown::on_autostartAppList_clicked(const QModelIndex &index)
{
    App app = index.data(Qt::UserRole + 3).value<App>();

    ui->autostartAppName->setText(app.name());
    ui->autostartAppCommand->setText(app.command().trimmed());
    ui->autostartInTheshell->setChecked(false);

    ui->startupStack->setCurrentIndex(2);
}

void InfoPaneDropdown::on_enterCommandAutoStartApps_clicked()
{
    ui->autostartAppName->setText("");
    ui->autostartAppCommand->setText("");
    ui->autostartInTheshell->setChecked(false);
    ui->startupStack->setCurrentIndex(2);
}

void InfoPaneDropdown::on_addAutostartApp_clicked()
{
    QString desktopEntryData;
    desktopEntryData.append("[Desktop Entry]\n");
    desktopEntryData.append("Type=Application\n");
    desktopEntryData.append("Version=1.0\n");
    desktopEntryData.append("Name=" + ui->autostartAppName->text() + "\n");
    desktopEntryData.append("Exec=" + ui->autostartAppCommand->text() + "\n");
    desktopEntryData.append("Terminal=false\n");
    if (ui->autostartInTheshell->isChecked()) {
        desktopEntryData.append("OnlyShowIn=theshell;");
    }

    QFile desktopEntry(QDir::homePath() + "/.config/autostart/" + ui->autostartAppName->text().toLower().replace(" ", "_").append(".desktop"));

    if (desktopEntry.exists()) {
        if (QMessageBox::warning(this, "Autostart Definition", "There is already an autostart definition for this app. Do you want to overwrite it?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            return;
        }
    }

    desktopEntry.open(QFile::WriteOnly);
    desktopEntry.write(desktopEntryData.toUtf8());
    desktopEntry.close();

    updateAutostart();
    ui->startupStack->setCurrentIndex(0);
}

void InfoPaneDropdown::on_redshiftSwitch_toggled(bool checked)
{
    if (effectiveRedshiftOn) {
        if (checked) { //Turn Redshift back on
            overrideRedshift = 0;
        } else { //Temporarily disable Redshift
            overrideRedshift = 1;
        }
    } else {
        if (checked) { //Temporarily enable Redshift
            overrideRedshift = 2;
        } else { //Turn Redshift back off
            overrideRedshift = 0;
        }
    }
}

void InfoPaneDropdown::on_grayColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "gray");
        updateAccentColourBox();
        resetStyle();
        changeDropDown(Settings, false);
    }
}

void InfoPaneDropdown::on_AppNotifications_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == NULL) {
        ui->appNotificationsPane->setEnabled(false);
        ui->appNotificationsConfigureLock->setVisible(false);
    } else {
        ui->appNotificationsTitle->setText(tr("Notifications for %1").arg(current->text()));
        ui->appAllowNotifications->setChecked(notificationAppSettings->value(current->text() + "/allow", true).toBool());
        ui->appAllowSounds->setChecked(notificationAppSettings->value(current->text() + "/sounds", true).toBool());
        ui->appAllowPopup->setChecked(notificationAppSettings->value(current->text() + "/popup", true).toBool());
        ui->appBypassQuiet->setChecked(notificationAppSettings->value(current->text() + "/bypassQuiet", false).toBool());

        if (current->text() == "theShell") {
            ui->appNotificationsPane->setEnabled(false);
            ui->appNotificationsConfigureLock->setText(tr("You can't configure notifications for %1").arg(current->text()));
            ui->appNotificationsConfigureLock->setVisible(true);
        } else {
            ui->appNotificationsPane->setEnabled(true);
            ui->appNotificationsConfigureLock->setVisible(false);
        }
    }
}

void InfoPaneDropdown::on_appAllowNotifications_toggled(bool checked)
{
    if (ui->AppNotifications->currentItem() != NULL) {
        notificationAppSettings->setValue(ui->AppNotifications->currentItem()->text() + "/allow", checked);
    }
}

void InfoPaneDropdown::on_appAllowSounds_toggled(bool checked)
{
    if (ui->AppNotifications->currentItem() != NULL) {
        notificationAppSettings->setValue(ui->AppNotifications->currentItem()->text() + "/sounds", checked);
    }
}

void InfoPaneDropdown::on_appAllowPopup_toggled(bool checked)
{
    if (ui->AppNotifications->currentItem() != NULL) {
        notificationAppSettings->setValue(ui->AppNotifications->currentItem()->text() + "/popup", checked);
    }
}

void InfoPaneDropdown::on_appBypassQuiet_toggled(bool checked)
{
    if (ui->AppNotifications->currentItem() != NULL) {
        notificationAppSettings->setValue(ui->AppNotifications->currentItem()->text() + "/bypassQuiet", checked);
    }
}

void InfoPaneDropdown::on_SetSystemTimezoneButton_clicked()
{
    ui->TimezoneStackedWidget->setCurrentIndex(1);

    launchDateTimeService();
    QDBusInterface dateTimeInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QString currentTimezone = dateTimeInterface.property("Timezone").toString();

    timezoneData = QJsonObject();

    ui->timezoneList->clear();
    QFile tzInfo("/usr/share/zoneinfo/zone.tab");
    tzInfo.open(QFile::ReadOnly);
    while (!tzInfo.atEnd()) {
        QString tzLine = tzInfo.readLine();
        if (!tzLine.startsWith("#")) {
            QStringList parts = tzLine.trimmed().split("\t", QString::SkipEmptyParts);
            if (parts.length() >= 3) {
                QString region = parts.at(2).left(parts.at(2).indexOf("/"));
                QString city = parts.at(2).mid(parts.at(2).indexOf("/") + 1);

                if (!timezoneData.contains(region)) {
                    QListWidgetItem* i = new QListWidgetItem();
                    i->setText(region);
                    ui->timezoneList->addItem(i);
                    timezoneData.insert(region, QJsonArray());
                }

                QJsonObject cityData;
                cityData.insert("name", city);
                cityData.insert("country", parts.at(0).toLower());
                cityData.insert("descriptor", parts.at(2));
                if (parts.at(2) == currentTimezone) {
                    cityData.insert("selected", true);
                } else {
                    cityData.insert("selected", false);
                }

                QJsonArray a = timezoneData.value(region).toArray();
                a.append(cityData);
                timezoneData.insert(region, a);
            }
        }
    }
    tzInfo.close();

    ui->setTimezoneButton->setEnabled(false);
    ui->timezoneCityList->clear();
}

void InfoPaneDropdown::on_backTimezone_clicked()
{
    ui->TimezoneStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_setTimezoneButton_clicked()
{
    ui->TimezoneStackedWidget->setCurrentIndex(0);

    //Set the timezone
    LOWER_INFOPANE
    launchDateTimeService();
    QDBusInterface dateTimeInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QDBusPendingCallWatcher* w = new QDBusPendingCallWatcher(dateTimeInterface.asyncCall("SetTimezone", ui->timezoneCityList->currentItem()->data(Qt::UserRole), true));
    connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), w, SLOT(deleteLater()));
    connect(w, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(updateDSTNotification()));
}

void InfoPaneDropdown::on_timezoneList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->timezoneCityList->clear();
    if (current != NULL) {
        QJsonArray a = timezoneData.value(current->text()).toArray();
        for (QJsonValue v : a) {
            QListWidgetItem* i = new QListWidgetItem();
            QJsonObject cityData = v.toObject();
            i->setText(cityData.value("name").toString().replace("_", " "));
            i->setData(Qt::UserRole, cityData.value("descriptor").toString());
            i->setIcon(QIcon::fromTheme("flag-" + cityData.value("country").toString(), QIcon::fromTheme("flag")));
            ui->timezoneCityList->addItem(i);
            if (cityData.value("selected").toBool()) {
                i->setSelected(true);
            }
        }
    }
}

void InfoPaneDropdown::on_timezoneCityList_currentRowChanged(int currentRow)
{
    if (currentRow == -1) {
        ui->setTimezoneButton->setEnabled(false);
    } else {
        ui->setTimezoneButton->setEnabled(true);
    }
}

void InfoPaneDropdown::on_batteryScreenOff_valueChanged(int value)
{
    settings.setValue("power/batteryScreenOff", value);
    if (value == 121) {
        ui->batteryScreenOffLabel->setText(tr("Never"));
    } else {
        ui->batteryScreenOffLabel->setText(tr("%n min(s)", NULL, value));
    }
}

void InfoPaneDropdown::on_batterySuspend_valueChanged(int value)
{
    settings.setValue("power/batterySuspend", value);
    if (value == 121) {
        ui->batterySuspendLabel->setText(tr("Never"));
    } else {
        ui->batterySuspendLabel->setText(tr("%n min(s)", NULL, value));
    }
}

void InfoPaneDropdown::on_powerScreenOff_valueChanged(int value)
{
    settings.setValue("power/powerScreenOff", value);
    if (value == 121) {
        ui->powerScreenOffLabel->setText(tr("Never"));
    } else {
        ui->powerScreenOffLabel->setText(tr("%n min(s)", NULL, value));
    }
}

void InfoPaneDropdown::on_powerSuspend_valueChanged(int value)
{
    settings.setValue("power/powerSuspend", value);
    if (value == 121) {
        ui->powerSuspendLabel->setText(tr("Never"));
    } else {
        ui->powerSuspendLabel->setText(tr("%n min(s)", NULL, value));
    }
}

void InfoPaneDropdown::on_quietModeExpandButton_clicked()
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->quietModeExtras->height());
    if (ui->quietModeExtras->height() == 0) {
        anim->setEndValue(ui->quietModeExtras->sizeHint().height());
        ui->quietModeExpandButton->setIcon(QIcon::fromTheme("go-up"));
    } else {
        anim->setEndValue(0);
        ui->quietModeExpandButton->setIcon(QIcon::fromTheme("go-down"));
    }
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->quietModeExtras->setFixedHeight(value.toInt());
    });
    anim->start();
}

void InfoPaneDropdown::on_quietModeForeverButton_toggled(bool checked)
{
    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(false);
        ui->quietModeTurnOffInTimer->setVisible(false);
        AudioMan->setQuietModeResetTime(QDateTime::fromString(""));
    }
}

void InfoPaneDropdown::on_quietModeTurnOffIn_toggled(bool checked)
{
    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(false);
        ui->quietModeTurnOffInTimer->setVisible(true);

        QDateTime oneHour = QDateTime::currentDateTime().addSecs(3600);
        AudioMan->setQuietModeResetTime(oneHour);
        ui->quietModeTurnOffInTimer->setTime(QTime(1, 0));
    }
}

void InfoPaneDropdown::on_quietModeTurnOffAt_toggled(bool checked)
{

    if (checked) {
        ui->quietModeTurnOffAtTimer->setVisible(true);
        ui->quietModeTurnOffInTimer->setVisible(false);

        QDateTime oneHour = QDateTime::currentDateTime().addSecs(3600);
        AudioMan->setQuietModeResetTime(oneHour);
        ui->quietModeTurnOffAtTimer->setDateTime(oneHour);
    }
}

void InfoPaneDropdown::on_quietModeTurnOffAtTimer_editingFinished()
{
    AudioMan->setQuietModeResetTime(ui->quietModeTurnOffAtTimer->dateTime());
}

void InfoPaneDropdown::on_quietModeTurnOffInTimer_editingFinished()
{
    QDateTime timeout = QDateTime::currentDateTime().addMSecs(ui->quietModeTurnOffInTimer->time().msecsSinceStartOfDay());
    AudioMan->setQuietModeResetTime(timeout);
}

void InfoPaneDropdown::on_removeAutostartButton_clicked()
{
    if (ui->autostartList->currentItem() != nullptr) {
        tToast* toast = new tToast();
        toast->setText("Autostart item has been removed.");
        toast->setTitle("Remove Autostart Item");

        QMap<QString, QString> actions;
        actions.insert("undo", "Undo");
        toast->setActions(actions);

        QListWidgetItem* i = ui->autostartList->takeItem(ui->autostartList->currentIndex().row());
        bool* deleteItem = new bool(true);

        connect(toast, &tToast::dismissed, [=] {
            if (*deleteItem) {
                QFile(i->data(Qt::UserRole).toString()).remove();
                delete i;
            }
            delete deleteItem;
            toast->deleteLater();
        });
        connect(toast, &tToast::actionClicked, [=](QString key) {
            *deleteItem = false;
            ui->autostartList->addItem(i);
        });
        toast->show(this);
    }
}

void InfoPaneDropdown::on_resetDeviceButton_clicked()
{
    QProcess::startDetached("scallop --reset");
    this->close();
}

void InfoPaneDropdown::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}

void InfoPaneDropdown::on_sunlightRedshift_toggled(bool checked)
{
    settings.setValue("display/redshiftSunlightCycle", checked);
    updateRedshiftTime();
}

void InfoPaneDropdown::updateRedshiftTime() {
    if (!settings.value("display/redshiftSunlightCycle", false).toBool()) {
        //Don't grab location if user doesn't want
        ui->startRedshift->setEnabled(true);
        ui->endRedshift->setEnabled(true);
        return;
    }
    ui->startRedshift->setEnabled(false);
    ui->endRedshift->setEnabled(false);

    geolocation->singleShot()->then([=](Geolocation loc) {
        QNetworkAccessManager* manager = new QNetworkAccessManager();
        QNetworkRequest sunriseApi(QUrl(QString("https://api.sunrise-sunset.org/json?lat=%1&lng=%2&formatted=0").arg(loc.latitude).arg(loc.longitude)));
        sunriseApi.setHeader(QNetworkRequest::UserAgentHeader, QString("theShell/%1").arg(TS_VERSION));

        QNetworkReply* reply = manager->get(sunriseApi);
        connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code){
            qDebug() << "Error";
        });
        connect(reply, &QNetworkReply::finished, [=] {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject root = doc.object();
            if (root.value("status").toString() != "OK") {
                qDebug() << root.value("status").toString();
                return;
            }

            //The time returned should be midway into the transition period, add/remove 30 minutes to compensate
            QJsonObject results = root.value("results").toObject();
            QDateTime sunrise = QDateTime::fromString(results.value("sunrise").toString(), Qt::ISODate).toLocalTime().addSecs(-1800);
            QDateTime sunset = QDateTime::fromString(results.value("sunset").toString(), Qt::ISODate).toLocalTime().addSecs(1800);

            ui->startRedshift->setDateTime(sunset);
            ui->endRedshift->setDateTime(sunrise);

            reply->deleteLater();
            manager->deleteLater();
        });
    });
}

void InfoPaneDropdown::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    if (settings.value("bar/onTop", true).toBool()) {
        painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
    } else {
        painter.drawLine(0, 0, this->width(), 0);
    }
    event->accept();
}

void InfoPaneDropdown::on_systemGTK3Theme_currentIndexChanged(int index)
{
    gtk3Settings->setValue("Settings/gtk-theme-name", ui->systemGTK3Theme->itemText(index));
}

void InfoPaneDropdown::on_systemFontSize_valueChanged(int arg1)
{
    themeSettings->setValue("font/defaultSize", arg1);
}

void InfoPaneDropdown::on_systemGTK3Font_currentFontChanged(const QFont &f)
{
    gtk3Settings->setValue("Settings/gtk-font-name", f.family() + " " + QString::number(ui->systemGTK3FontSize->value()));
}

void InfoPaneDropdown::on_systemGTK3FontSize_valueChanged(int arg1)
{
    gtk3Settings->setValue("Settings/gtk-font-name", ui->systemGTK3Font->currentFont().family() + " " + QString::number(arg1));
}

void InfoPaneDropdown::on_useSystemFontForGTKButton_clicked()
{
    ui->systemGTK3Font->setCurrentFont(ui->systemFont->currentFont());
    ui->systemGTK3FontSize->setValue(ui->systemFontSize->value());
}

void InfoPaneDropdown::setHeaderColour(QColor col) {
    if (ui->HighContrastSwitch->isChecked()) {
        ui->partFrame->setPalette(QApplication::palette(ui->partFrame));
    } else {
        QPalette pal = ui->partFrame->palette();

        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(pal.color(QPalette::Window));
        anim->setEndValue(col);
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            QPalette pal = ui->partFrame->palette();
            QColor col = value.value<QColor>();

            pal.setColor(QPalette::Window, col);

            //if ((col.red() + col.green() + col.blue()) / 3 < 127) {
                pal.setColor(QPalette::WindowText, Qt::white);
                pal.setColor(QPalette::Disabled, QPalette::WindowText, col.lighter(150));
            /*} else {
                pal.setColor(QPalette::WindowText, Qt::black);
                pal.setColor(QPalette::Disabled, QPalette::WindowText, col.darker(150));
            }*/

            ui->partFrame->setPalette(pal);

            pal.setColor(QPalette::Window, col.lighter(120));
            ui->pushButton_7->setPalette(pal);

            for (int i = 0; i < ui->InformationalPluginsLayout->count(); i++) {
                ClickableLabel* l = (ClickableLabel*) ui->InformationalPluginsLayout->itemAt(i)->widget();
                l->setShowDisabled(l->showDisabled());
            }
        });
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();
    }
}

bool InfoPaneDropdown::eventFilter(QObject *obj, QEvent *e) {
    if (obj == ui->partFrame) {
        if (e->type() == QEvent::Paint) {
            QPainter p(ui->partFrame);
            QPalette pal = ui->partFrame->palette();

            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(pal.color(QPalette::Window));
            p.setPen(Qt::transparent);
            p.drawRect(0, 0, ui->partFrame->width(), ui->partFrame->height());

            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                int width = ui->partFrame->width();
                QPolygonF firstPoly;
                firstPoly.append(QPointF(width - slice1.currentValue().toFloat(), 0));
                firstPoly.append(QPointF(width - slice2.currentValue().toFloat(), ui->partFrame->height()));
                firstPoly.append(QPointF(0, ui->partFrame->height()));
                firstPoly.append(QPointF(0, 0));
                p.setBrush(pal.color(QPalette::Window).lighter(110));
                p.drawPolygon(firstPoly);

                QPolygonF secondPoly;
                secondPoly.append(QPointF(width - ui->partFrame->width() * 0.85, 0));
                secondPoly.append(QPointF(width - ui->partFrame->width() * 0.825, ui->partFrame->height()));
                secondPoly.append(QPointF(0, ui->partFrame->height()));
                secondPoly.append(QPointF(0, 0));
                p.setBrush(pal.color(QPalette::Window).lighter(120));
                p.drawPolygon(secondPoly);
            } else {
                QPolygonF firstPoly;
                firstPoly.append(QPointF(slice1.currentValue().toFloat(), 0));
                firstPoly.append(QPointF(slice2.currentValue().toFloat(), ui->partFrame->height()));
                firstPoly.append(QPointF(ui->partFrame->width(), ui->partFrame->height()));
                firstPoly.append(QPointF(ui->partFrame->width(), 0));
                p.setBrush(pal.color(QPalette::Window).lighter(110));
                p.drawPolygon(firstPoly);

                QPolygonF secondPoly;
                secondPoly.append(QPointF(ui->partFrame->width() * 0.85, 0));
                secondPoly.append(QPointF(ui->partFrame->width() * 0.825, ui->partFrame->height()));
                secondPoly.append(QPointF(ui->partFrame->width(), ui->partFrame->height()));
                secondPoly.append(QPointF(ui->partFrame->width(), 0));
                p.setBrush(pal.color(QPalette::Window).lighter(120));
                p.drawPolygon(secondPoly);
            }
            return true;
        }
    }
    return false;
}

void InfoPaneDropdown::on_EmphasiseAppSwitch_toggled(bool checked)
{
    settings.setValue("notifications/emphasiseApp", checked);
}

void InfoPaneDropdown::on_CompactBarSwitch_toggled(bool checked)
{
    if (settings.value("bar/compact") != checked) {
        settings.setValue("bar/compact", checked);

        QMap<QString, QString> actions;
        actions.insert("logout", tr("Log Out Now"));

        tToast* t = new tToast();
        t->setTitle(tr("Logoff Required"));

        if (checked) {
            t->setText(tr("In order to enable the Compact Bar, you'll need to log out and then log back on."));
        } else {
            t->setText(tr("In order to disable the Compact Bar, you'll need to log out and then log back on."));
        }
        t->setActions(actions);
        t->setTimeout(10000);
        connect(t, &tToast::actionClicked, [=](QString key) {
            if (key == "logout") {
                EndSession(EndSessionWait::logout);
            }
        });
        connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
        t->show(this);
    }
}

void InfoPaneDropdown::keyPressEvent(QKeyEvent *event) {

}

void InfoPaneDropdown::on_blackColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "black");
        updateAccentColourBox();
        resetStyle();
        changeDropDown(Settings, false);
    }
}

void InfoPaneDropdown::changeSettingsPane(int pane) {
    ui->settingsList->setCurrentRow(pane);
}

void InfoPaneDropdown::on_allowGeoclueAgent_clicked()
{
    //Automatically edit the geoclue file
    LOWER_INFOPANE
    QProcess::execute("pkexec sed \"/whitelist=.*/ s/$/;theshell/\" -i /etc/geoclue/geoclue.conf");
    locationServices->reloadAuthorizationRequired();

    QTimer::singleShot(500, [=] {
        on_settingsList_currentRowChanged(6);
    });
}

void InfoPaneDropdown::on_LocationMasterSwitch_toggled(bool checked)
{
    locationSettings->setValue("master/master", checked);
}

void InfoPaneDropdown::setupLocationSettingsPane() {
    if (locationServices->requiresAuthorization()) {
        ui->locationStack->setCurrentIndex(0);
    } else {
        ui->locationStack->setCurrentIndex(1);

        ui->LocationAppsList->clear();
        QStringList availableApps = locationSettings->childGroups();
        availableApps.removeOne("master");

        for (QString app : availableApps) {
            locationSettings->beginGroup(app);
            bool allow = locationSettings->value("allow").toBool();

            App a = App::invalidApp();
            if (QFile("/usr/share/applications/" + app + ".desktop").exists()) {
                a = AppsListModel::readAppFile("/usr/share/applications/" + app + ".desktop");
            } else if (QFile(QDir::homePath() + "/.local/share/applications" + app + ".desktop").exists()) {
                a = AppsListModel::readAppFile(QDir::homePath() + "/.local/share/applications" + app + ".desktop");
            }

            QListWidgetItem* i = new QListWidgetItem();
            if (a.invalid()) {
                i->setText(app);
            } else {
                i->setIcon(a.icon());
                i->setText(a.name());
            }

            i->setFlags(i->flags() | Qt::ItemIsUserCheckable);
            if (allow) {
                i->setCheckState(Qt::Checked);
            } else {
                i->setCheckState(Qt::Unchecked);
            }
            i->setData(Qt::UserRole, app);
            ui->LocationAppsList->addItem(i);

            locationSettings->endGroup();
        }
    }
}

void InfoPaneDropdown::on_LocationAppsList_itemChanged(QListWidgetItem *item)
{
    if (item->checkState() == Qt::Checked) {
        locationSettings->setValue(item->data(Qt::UserRole).toString() + "/allow", true);
    } else {
        locationSettings->setValue(item->data(Qt::UserRole).toString() + "/allow", false);
    }
}

void InfoPaneDropdown::on_backInput_clicked()
{
    ui->InputStack->setCurrentIndex(0);
}

void InfoPaneDropdown::on_addLayout_clicked()
{
    ui->InputStack->setCurrentIndex(1);
}

void InfoPaneDropdown::on_addKeyboardLayout_clicked()
{
    QListWidgetItem* item = ui->availableKeyboardLayouts->currentItem();
    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    currentLayouts.append(item->data(Qt::UserRole).toString());

    QListWidgetItem* newItem = new QListWidgetItem();
    newItem->setText(item->text());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
    ui->selectedLayouts->addItem(newItem);
    settings.setValue("input/layout", currentLayouts.join(","));

    ui->InputStack->setCurrentIndex(0);
    loadNewKeyboardLayoutMenu();
}

void InfoPaneDropdown::on_removeLayout_clicked()
{
    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    int item = ui->selectedLayouts->currentRow();
    currentLayouts.removeAt(item);
    QListWidgetItem* i = ui->selectedLayouts->takeItem(item);
    delete i;
    settings.setValue("input/layout", currentLayouts.join(","));
    loadNewKeyboardLayoutMenu();
}

void InfoPaneDropdown::on_selectedLayouts_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr) {
        ui->removeLayout->setEnabled(false);
    } else if (ui->selectedLayouts->count() == 1) {
        ui->removeLayout->setEnabled(false);
    } else {
        ui->removeLayout->setEnabled(true);
    }
}

void InfoPaneDropdown::on_availableKeyboardLayouts_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr) {
        ui->addKeyboardLayout->setEnabled(false);
    } else {
        ui->addKeyboardLayout->setEnabled(true);
    }
}

void InfoPaneDropdown::KeyboardLayoutsMoved()
{
    //Completely recreate the selected layouts
    QStringList currentLayouts;
    for (int i = 0; i < ui->selectedLayouts->count(); i++) {
        QListWidgetItem* item = ui->selectedLayouts->item(i);
        currentLayouts.append(item->data(Qt::UserRole).toString());
    }
    settings.setValue("input/layout", currentLayouts.join(","));
    loadNewKeyboardLayoutMenu();
}

void InfoPaneDropdown::on_moveLayoutDown_clicked()
{
    int row = ui->selectedLayouts->currentRow();
    if (row <= ui->selectedLayouts->count() - 1) {
        ui->selectedLayouts->clearSelection();
        QListWidgetItem* item = ui->selectedLayouts->takeItem(row);
        ui->selectedLayouts->insertItem(row + 1, item);
        item->setSelected(true);
    }

    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    QString layout = currentLayouts.takeAt(row);
    currentLayouts.insert(row + 1, layout);
    settings.setValue("input/layout", currentLayouts.join(","));
    loadNewKeyboardLayoutMenu();
}

void InfoPaneDropdown::on_moveLayoutUp_clicked()
{
    int row = ui->selectedLayouts->currentRow();
    if (row > 0) {
        ui->selectedLayouts->clearSelection();
        QListWidgetItem* item = ui->selectedLayouts->takeItem(row);
        ui->selectedLayouts->insertItem(row - 1, item);
        item->setSelected(true);
    }

    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    QString layout = currentLayouts.takeAt(row);
    currentLayouts.insert(row - 1, layout);
    settings.setValue("input/layout", currentLayouts.join(","));
    loadNewKeyboardLayoutMenu();
}

void InfoPaneDropdown::loadNewKeyboardLayoutMenu() {
    //Check to see if current keyboard layout is included in list, and if not, select first
    QString currentLayout = settings.value("input/currentLayout", "us(basic)").toString();
    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    if (!currentLayouts.contains(currentLayout)) {
        setKeyboardLayout(currentLayouts.first());
        return;
    }

    if (ui->selectedLayouts->count() == 1) {
        emit newKeyboardLayoutMenuAvailable(nullptr);
    } else {
        QMenu* menu = new QMenu();
        menu->addSection(tr("Keyboard Layout"));
        for (int i = 0; i < ui->selectedLayouts->count(); i++) {
            QListWidgetItem* item = ui->selectedLayouts->item(i);
            QAction* action = menu->addAction(item->text(), [=] {
                setKeyboardLayout(item->data(Qt::UserRole).toString());
            });
            action->setCheckable(true);
            if (item->data(Qt::UserRole) == currentLayout) {
                action->setChecked(true);
            }
        }
        emit newKeyboardLayoutMenuAvailable(menu);
    }
    QApplication::processEvents();
}

void InfoPaneDropdown::setKeyboardLayout(QString layout) {
    settings.setValue("input/currentLayout", layout);
    QProcess::startDetached("setxkbmap " + layout);
    loadNewKeyboardLayoutMenu();
    emit keyboardLayoutChanged(layout.split("(").first().toUpper());
}

QString InfoPaneDropdown::setNextKeyboardLayout() {
    QString currentLayout = settings.value("input/currentLayout", "us(basic)").toString();
    QStringList currentLayouts = settings.value("input/layout", "us(basic)").toString().split(",");
    int currentIndex = currentLayouts.indexOf(currentLayout);
    currentIndex++;
    if (currentIndex == currentLayouts.count()) currentIndex = 0;

    QString layout = currentLayouts.at(currentIndex);
    QTimer::singleShot(0, [=] {
        setKeyboardLayout(layout);
    });
    for (int i = 0; i < ui->selectedLayouts->count(); i++) {
        if (ui->selectedLayouts->item(i)->data(Qt::UserRole) == layout) {
            return ui->selectedLayouts->item(i)->text();
        }
    }
}

void InfoPaneDropdown::on_setupMousePassword_clicked()
{
    //Check Polkit authorization
    PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.thesuite.theshell.configure-mouse-password", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);

    if (r == PolkitQt1::Authority::No) {
        QMessageBox::warning(this, tr("Unauthorized"), tr("Polkit does not allow you to set up a mouse password."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    } else if (r == PolkitQt1::Authority::Challenge) {
        LOWER_INFOPANE
        PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.thesuite.theshell.configure-mouse-password", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::AllowUserInteraction);
        if (r != PolkitQt1::Authority::Yes) {
            return;
        }
    }

    ui->lockScreenStack->setCurrentIndex(1);
}

void InfoPaneDropdown::on_removeMousePassword_clicked()
{
    if (QMessageBox::question(this, tr("Remove Mouse Password?"), tr("Do you want to remove the Mouse Password for this account?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        //Check Polkit authorization
        PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.thesuite.theshell.configure-mouse-password", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);

        if (r == PolkitQt1::Authority::No) {
            QMessageBox::warning(this, tr("Unauthorized"), tr("Polkit does not allow you to set up a mouse password."), QMessageBox::Ok, QMessageBox::Ok);
            return;
        } else if (r == PolkitQt1::Authority::Challenge) {
            LOWER_INFOPANE
            PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.thesuite.theshell.configure-mouse-password", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::AllowUserInteraction);
            if (r != PolkitQt1::Authority::Yes) {
                return;
            }
        }


        //Remove the mouse password
        QProcess* proc = new QProcess();
        QDir::home().mkdir(".theshell");

        QString executable = "/usr/lib/ts-mousepass-change";
        #ifdef BLUEPRINT
            executable += "b";
        #endif
        proc->start(executable + " --remove --passfile=" + QDir::homePath() + "/.theshell/mousepassword");
        proc->waitForFinished();

        if (proc->exitCode() == 0) {
            tToast* toast = new tToast();
            toast->setTitle(tr("Mouse Password"));
            toast->setText(tr("Mouse Password was removed successfully"));
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this);
        } else {
            tToast* toast = new tToast();
            toast->setTitle(tr("Mouse Password"));
            toast->setText(tr("Mouse Password couldn't be removed"));
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this);
        }

        proc->deleteLater();
    }
}

void InfoPaneDropdown::on_MousePasswordSetup_exit()
{
    ui->lockScreenStack->setCurrentIndex(0);
    if (QFile(QDir::homePath() + "/.theshell/mousepassword").exists()) {
        ui->removeMousePassword->setVisible(true);
    } else {
        ui->removeMousePassword->setVisible(false);
    }
}

void InfoPaneDropdown::on_websiteButton_clicked()
{
    QProcess::startDetached("xdg-open https://vicr123.github.io/theshell");
    this->close();
}

void InfoPaneDropdown::on_bugButton_clicked()
{
    QProcess::startDetached("xdg-open https://github.com/vicr123/theshell/issues");
    this->close();
}

void InfoPaneDropdown::on_distroWebpage_clicked()
{
    QProcess::startDetached("xdg-open " + ui->distroWebpage->text());
    this->close();
}

void InfoPaneDropdown::on_distroSupport_clicked()
{
    QProcess::startDetached("xdg-open " + ui->distroSupport->text());
    this->close();
}

void InfoPaneDropdown::on_sourcesButton_clicked()
{
    QProcess::startDetached("xdg-open https://github.com/vicr123/theshell");
    this->close();
}

void InfoPaneDropdown::pluginMessage(QString message, QVariantList args, StatusCenterPaneObject* caller) {
    if (message == "main-menu") {
        if (ui->settingsListContainer->width() == 0) pluginMessage("show-menu", QVariantList(), caller);
        ui->settingsListStack->setCurrentIndex(0);
    } else if (message == "hide-menu") {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->settingsListContainer->width());
        anim->setEndValue(0);
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->settingsListContainer->setFixedWidth(value.toInt());
        });
        anim->start(tVariantAnimation::DeleteWhenStopped);
    } else if (message == "show-menu") {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->settingsListContainer->width());
        anim->setEndValue((int) (250 * getDPIScaling()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->settingsListContainer->setFixedWidth(value.toInt());
        });
        anim->start(tVariantAnimation::DeleteWhenStopped);
    } else if (message == "register-switch") {
        uint thisSwitchId = pluginSwitches.count();
        QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);

        QLabel* switchLabel = new QLabel();
        switchLabel->setText(args.first().toString());
        layout->addWidget(switchLabel);

        Switch* s = new Switch();
        connect(s, &Switch::toggled, [=](bool checked) {
            caller->message("switch-toggled", QVariantList() << thisSwitchId << checked);
        });
        if (args.count() >= 2) {
            s->setChecked(args.at(1).toBool());
        }
        layout->addWidget(s);
        pluginSwitches.append(s);

        ui->customSwitches->addLayout(layout);

        caller->message("switch-registered", QVariantList() << thisSwitchId << args.first().toString());
    } else if (message == "toggle-switch") {
        Switch* s = pluginSwitches.value(args.first().toUInt());
        s->setChecked(args.last().toBool());
    } else if (message == "attenuate") {
        if (args.first() == true) {
            AudioMan->attenuateStreams();
        } else {
            AudioMan->restoreStreams();
        }
    } else if (message == "location") {
        geolocation->singleShot()->then([=](Geolocation loc) {
            caller->message("location", QVariantList() << loc.latitude << loc.longitude);
        });
    } else if (message == "jobDone") {
        emit statusBarProgressFinished(args.first().toString(), args.at(1).toString());
    } else if (message == "jobUpdate") {
        emit statusBarProgress(args.first().toString(), args.at(1).toString(), args.at(2).toUInt());
    }
}

void InfoPaneDropdown::on_settingsList_itemActivated(QListWidgetItem *item)
{
    QVariant setting = item->data(Qt::UserRole);
    if (!setting.isNull() && setting.toInt() != -1) {
        ui->settingsListStack->setCurrentIndex(setting.toInt());
    }
}

void InfoPaneDropdown::on_quietModeCriticalOnly_clicked()
{
    AudioMan->setQuietMode(AudioManager::critical);
    ui->quietModeCriticalOnly->setChecked(true);
}

void InfoPaneDropdown::on_powerSuspendNormally_toggled(bool checked)
{
    if (checked) {
        settings.setValue("power/suspendMode", 0);
    }
}

void InfoPaneDropdown::on_powerSuspendTurnOffScreen_toggled(bool checked)
{
    if (checked) {
        settings.setValue("power/suspendMode", 1);
    }
}

void InfoPaneDropdown::on_powerSuspendHibernate_toggled(bool checked)
{
    if (checked) {
        settings.setValue("power/suspendMode", 2);
    }
}

void InfoPaneDropdown::on_powerButtonPressed_currentIndexChanged(int index)
{
    settings.setValue("power/onPowerButtonPressed", index);
}
