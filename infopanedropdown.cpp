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

#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"
#include "internationalisation.h"

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

InfoPaneDropdown::InfoPaneDropdown(WId MainWindowId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    ui->setupUi(this);

    ui->settingsList->setIconSize(QSize(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->settingsList->setFixedWidth(250 * getDPIScaling());

    startTime.start();

    if (settings.value("flightmode/on", false).toBool()) {
        ui->FlightSwitch->setChecked(true);
    }

    this->MainWindowId = MainWindowId;

    //ndbus->setDropdownPane(this);

    /*connect(ndbus, SIGNAL(newNotification(int,QString,QString,QIcon)), this, SLOT(newNotificationReceived(int,QString,QString,QIcon)));
    connect(ndbus, SIGNAL(removeNotification(int)), this, SLOT(removeNotification(int)));
    connect(ndbus, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(notificationClosed(uint,uint)));*/
    //connect(this, SIGNAL(closeNotification(int)), ndbus, SLOT(CloseNotificationUserInitiated(int)));
    connect(ndbus, SIGNAL(NotificationClosed(uint,uint)), this, SLOT(notificationClosed(uint,uint)));
    connect(ndbus, SIGNAL(ActionInvoked(uint,QString)), this, SLOT(notificationAction(uint,QString)));
    connect(ui->notificationsWidget, SIGNAL(numNotificationsChanged(int)), this, SIGNAL(numNotificationsChanged(int)));

    connect(dbusServiceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(DBusServiceRegistered(QString)));
    connect(dbusServiceWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(DBusServiceUnregistered(QString)));
    connect(dbusServiceWatcherSystem, SIGNAL(serviceRegistered(QString)), this, SLOT(DBusServiceRegistered(QString)));
    connect(dbusServiceWatcherSystem, SIGNAL(serviceUnregistered(QString)), this, SLOT(DBusServiceUnregistered(QString)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateSysInfo()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateKdeconnect()));
    connect(timer, &QTimer::timeout, [=]() {
        //Run the timer faster when a stopwatch is running.
        if (stopwatchRunning) {
            timer->setInterval(100);
        } else {
            timer->setInterval(1000);
        }
    });
    timer->setInterval(1000);
    timer->start();

    connect(updbus, &UPowerDBus::powerStretchChanged, [=](bool isOn) {
        ui->PowerStretchSwitch->setChecked(isOn);
        emit batteryStretchChanged(isOn);
        doNetworkCheck();
    });

    ui->label_7->setVisible(false);
    ui->pushButton_3->setVisible(false);
    ui->BatteryChargeScrollBar->setVisible(false);
    //ui->networkKey->setVisible(false);
    //ui->networkConnect->setVisible(false);
    ui->resetButton->setProperty("type", "destructive");
    ui->userSettingsDeleteUser->setProperty("type", "destructive");
    ui->userSettingsDeleteUserAndData->setProperty("type", "destructive");
    ui->userSettingsDeleteUserOnly->setProperty("type", "destructive");

    QPalette powerStretchPalette = ui->PowerStretchSwitch->palette();
    powerStretchPalette.setColor(QPalette::Highlight, QColor(255, 100, 0));
    powerStretchPalette.setColor(QPalette::WindowText, Qt::white);
    ui->PowerStretchSwitch->setPalette(powerStretchPalette);

    //Set up battery chart
    batteryChart = new QChart();
    batteryChart->setBackgroundVisible(false);
    batteryChart->legend()->hide();

    QChartView* batteryChartView = new QChartView(batteryChart);
    batteryChartView->setRenderHint(QPainter::Antialiasing);
    ((QBoxLayout*) ui->batteryGraph->layout())->insertWidget(1, batteryChartView);

    updateBatteryChart();

    //Set up KDE Connect
    if (!QFile("/usr/lib/kdeconnectd").exists()) {
        //If KDE Connect is not installed, hide the KDE Connect option
        ui->kdeconnectLabel->setVisible(false);
    }

    connect(ui->NetworkManager, SIGNAL(updateBarDisplay(QString,QIcon)), this, SIGNAL(networkLabelChanged(QString,QIcon)));

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
        QString currentWidgetTheme = settings.value("style/name", "contemporary").toString();
        QStringList keys = QStyleFactory::keys();
        for (QString key : keys) {
            ui->systemWidgetTheme->addItem(key);
            ui->systemWidgetTheme->setItemData(ui->systemWidgetTheme->count() - 1, key);

            if (key.toLower() == currentWidgetTheme.toLower()) {
                ui->systemWidgetTheme->setCurrentIndex(ui->systemWidgetTheme->count() - 1);
            }
        }
    }

    connect(this, &InfoPaneDropdown::networkLabelChanged, [=](QString label) {
        //ui->networkStatus->setText(label);
    });

    //ui->FlightSwitch->setOnIcon(QIcon::fromTheme("flight-mode"));

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

    QString thewaveVoiceEngine = settings.value("thewave/ttsEngine", "festival").toString();
    if (thewaveVoiceEngine == "pico2wave") {
        ui->thewaveTTSpico2wave->setChecked(true);
    } else if (thewaveVoiceEngine == "espeak") {
        ui->thewaveTTSespeak->setChecked(true);
    } else if (thewaveVoiceEngine == "festival") {
        ui->thewaveTTSfestival->setChecked(true);
    } else if (thewaveVoiceEngine == "none") {
        ui->thewaveTTSsilent->setChecked(true);
    }

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

    int dpi = sessionSettings->value("screen/dpi", 96).toInt();
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

    ui->lockScreenBackground->setText(lockScreenSettings->value("background", "/usr/share/icons/theos/backgrounds/triangle/1920x1080.png").toString());
    //ui->lineEdit_2->setText(settings.value("startup/autostart", "").toString());
    ui->redshiftPause->setChecked(!settings.value("display/redshiftPaused", true).toBool());
    ui->thewaveWikipediaSwitch->setChecked(settings.value("thewave/wikipediaSearch", true).toBool());
    ui->TouchFeedbackSwitch->setChecked(settings.value("input/touchFeedbackSound", false).toBool());
    ui->SuperkeyGatewaySwitch->setChecked(settings.value("input/superkeyGateway", true).toBool());
    ui->thewaveOffensiveSwitch->setChecked(settings.value("thewave/blockOffensiveWords", true).toBool());
    ui->theWaveSwitch->setChecked(settings.value("thewave/enabled", true).toBool());
    ui->theWaveName->setText(settings.value("thewave/name", "").toString());
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
    updateAccentColourBox();
    on_StatusBarSwitch_toggled(ui->StatusBarSwitch->isChecked());

    QString defaultFont;
    if (QFontDatabase().families().contains("Contemporary")) {
        defaultFont = "Contemporary";
    } else {
        defaultFont = "Noto Sans";
    }
    ui->systemFont->setFont(QFont(themeSettings->value("fonts/defaultFamily", defaultFont).toString(), themeSettings->value("font/defaultSize", 10).toInt()));

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

    /*do {
       for (QObject* object : allObjects) {
           if (object != NULL) {
               if (object->children().count() != 0) {
                   for (QObject* object2 : object->children()) {
                       allObjects.append(object2);
                   }
               }
               object->installEventFilter(this);
               allObjects.removeOne(object);
           }
       }
    } while (allObjects.count() != 0);*/

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
    }

    #ifdef BLUEPRINT
        ui->tsVersion->setText(tr("theShell %1 - Blueprint").arg(TS_VERSION));
        ui->compileDate->setText(tr("You compiled theShell on %1").arg(__DATE__));
    #else
        ui->tsVersion->setText(tr("theShell %1").arg(TS_VERSION));
        ui->compileDate->setVisible(false);
    #endif

    //Set up timer ringtones
    ringtone = new QMediaPlayer(this, QMediaPlayer::LowLatency);

    connect(NativeFilter, &NativeEventFilter::DoRetranslation, [=] {
        ui->retranslateUi(this);
    });

    connect(AudioMan, &AudioManager::QuietModeChanged, [=](AudioManager::quietMode mode) {
        ui->quietModeSound->setChecked(false);
        ui->quietModeNotification->setChecked(false);
        ui->quietModeMute->setChecked(false);

        if (mode == AudioManager::none) {
            ui->quietModeSound->setChecked(true);
            ui->quietModeSettings->setVisible(false);
        } else if (mode == AudioManager::notifications) {
            ui->quietModeNotification->setChecked(true);
            ui->quietModeSettings->setVisible(true);
            ui->quietModeDescription->setText(AudioMan->getCurrentQuietModeDescription());
        } else {
            ui->quietModeMute->setChecked(true);
            ui->quietModeSettings->setVisible(true);
            ui->quietModeDescription->setText(AudioMan->getCurrentQuietModeDescription());
        }
    });
    ui->quietModeSettings->setVisible(false);

    ui->RemindersList->setModel(new RemindersListModel);
    ui->RemindersList->setItemDelegate(new RemindersDelegate);

    /*AppsListModel* appsListModel = new AppsListModel();
    ui->autostartAppList->setModel(appsListModel);
    ui->autostartAppList->setItemDelegate(new AppsDelegate);*/

    updateStruts();
    updateAutostart();
}

InfoPaneDropdown::~InfoPaneDropdown()
{
    delete ui;
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
        QProcess* redshiftAdjust = new QProcess;
        connect(redshiftAdjust, SIGNAL(finished(int)), redshiftAdjust, SLOT(deleteLater()));
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

            redshiftAdjust->start("redshift -O " + QString::number(intensity));

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
            redshiftAdjust->start("redshift -O 6500");

            if (isRedshiftOn) {
                isRedshiftOn = false;
                effectiveRedshiftOn = false;
                ui->redshiftSwitch->setChecked(false);
                emit redshiftEnabledChanged(false);
            }
        }
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

void InfoPaneDropdown::timerTick() {
    ui->date->setText(QLocale().toString(QDateTime::currentDateTime(), "ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

    //Also update the stopwatch
    QTime stopwatchTime = QTime::fromMSecsSinceStartOfDay(0);
    stopwatchTime = stopwatchTime.addMSecs(stopwatchTimeAdd);
    if (stopwatchRunning) {
        stopwatchTime = stopwatchTime.addMSecs(this->stopwatchTime.elapsed());
    }
    ui->stopwatchLabel->setText(stopwatchTime.toString("hh:mm:ss.zzz"));
    updateTimers();

    //Also check for reminders
    {
        QList<QPair<QString, QDateTime>> ReminderData;

        QSettings reminders("theSuite/theShell.reminders");
        reminders.beginGroup("reminders");
        int count = reminders.beginReadArray("reminders");

        for (int i = 0; i < count; i++) {
            reminders.setArrayIndex(i);
            QPair<QString, QDateTime> data;
            data.first = reminders.value("title").toString();
            data.second = reminders.value("date").toDateTime();
            ReminderData.append(data);
        }

        reminders.endArray();

        bool dataChanged = false;
        for (int i = 0; i < ReminderData.count(); i++) {
            QPair<QString, QDateTime> data = ReminderData.at(i);
            if (data.second.msecsTo(QDateTime::currentDateTime()) > 0) {
                QVariantMap hints;
                hints.insert("category", "reminder.activate");
                hints.insert("sound-file", "qrc:/sounds/notifications/reminder.wav");
                ndbus->Notify("theShell", 0, "theshell", "Reminder", data.first, QStringList(), hints, 30000);
                ReminderData.removeAt(i);
                i--;
                dataChanged = true;
            }
        }


        if (dataChanged) {
            reminders.beginWriteArray("reminders");
            int i = 0;
            for (QPair<QString, QDateTime> data : ReminderData) {
                reminders.setArrayIndex(i);
                reminders.setValue("title", data.first);
                reminders.setValue("date", data.second);
                i++;
            }
            reminders.endArray();
            reminders.endGroup();

            ((RemindersListModel*) ui->RemindersList->model())->updateData();
        }
    }
}

void InfoPaneDropdown::show(dropdownType showWith) {
    changeDropDown(showWith, false);
    if (!this->isVisible()) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();

        if (settings.value("bar/onTop", true).toBool()) {
            this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), screenGeometry.width(), screenGeometry.height());
        } else {
            this->setGeometry(screenGeometry.x(), screenGeometry.bottom(), screenGeometry.width(), screenGeometry.height());
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
        this->setFixedHeight(screenGeometry.height());

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

    //Update the reminders list
    ((RemindersListModel*) ui->RemindersList->model())->updateData();
}

void InfoPaneDropdown::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());

    if (settings.value("bar/onTop", true).toBool()) {
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), this->width(), this->height()));
    } else {
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.bottom(), this->width(), this->height()));
    }
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, &tPropertyAnimation::finished, [=]() {
        QDialog::hide();
    });
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();
}

void InfoPaneDropdown::changeDropDown(dropdownType changeTo, bool doAnimation) {
    this->currentDropDown = changeTo;

    //Switch to the requested frame
    switch (changeTo) {
    case Clock:
        ui->pageStack->setCurrentWidget(ui->clockFrame, doAnimation);
        break;
    case Battery:
        ui->pageStack->setCurrentWidget(ui->statusFrame, doAnimation);
        updateBatteryChart();
        break;
    case Notifications:
        ui->pageStack->setCurrentWidget(ui->notificationsFrame, doAnimation);
        break;
    case Network:
        ui->pageStack->setCurrentWidget(ui->networkFrame, doAnimation);
        break;
    case KDEConnect:
        ui->pageStack->setCurrentWidget(ui->kdeConnectFrame, doAnimation);
        break;
    /*case Print:
        ui->pageStack->setCurrentWidget(ui->printFrame, doAnimation);
        break;*/
    case Settings:
        ui->pageStack->setCurrentWidget(ui->settingsFrame, doAnimation);
        break;
    }

    if (changeTo == Clock) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(true);
    } else if (changeTo == KDEConnect) { //Print) {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(false);
    } else if (changeTo == Settings) {
        ui->pushButton_5->setEnabled(false);
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
    changeDropDown(dropdownType(currentDropDown - 1));
}

void InfoPaneDropdown::on_pushButton_6_clicked()
{
    changeDropDown(dropdownType(currentDropDown + 1));
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

void InfoPaneDropdown::startTimer(QTime time) {
    if (timer != NULL) { //Check for already running timer
        timer->stop();
        delete timer;
        timer = NULL;
        ui->timeEdit->setVisible(true);
        ui->label_7->setVisible(false);
        ui->label_7->setEnabled(true);
        ui->pushButton_2->setText(tr("Start"));
        ui->pushButton_3->setVisible(false);
        emit timerVisibleChanged(false);
        emit timerEnabledChanged(true);
    }

    ui->pushButton_2->setText(tr("Pause"));
    timeUntilTimeout = time;
    lastTimer = time;
    ui->label_7->setText(ui->timeEdit->text());
    ui->timeEdit->setVisible(false);
    ui->label_7->setVisible(true);
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [=]() {
        timeUntilTimeout = timeUntilTimeout.addSecs(-1);
        if (timeUntilTimeout == QTime(0, 0, 0)) {
            if (timerNotificationId != 0) {
                ndbus->CloseNotification(timerNotificationId);
            }

            timer->stop();
            delete timer;
            timer = NULL;

            if (AudioMan->QuietMode() != AudioManager::notifications && AudioMan->QuietMode() != AudioManager::mute) { //Check if we should show the notification so the user isn't stuck listening to the tone
                QStringList actions;
                actions << "restart" << "Restart Timer";
                actions << "+0.5" << "+30 sec";
                actions << "+1" << "+1 min";
                actions << "+2" << "+2 min";
                actions << "+5" << "+5 min";
                actions << "+10" << "+10 min";

                QVariantMap hints;
                hints.insert("x-thesuite-timercomplete", true);
                hints.insert("suppress-sound", true);
                timerNotificationId = ndbus->Notify("theShell", 0, "", tr("Timer Elapsed"),
                                          tr("Your timer has completed."),
                                          actions, hints, 0);
                ui->timeEdit->setVisible(true);
                ui->label_7->setVisible(false);
                ui->pushButton_2->setText("Start");

                QMediaPlaylist* playlist = new QMediaPlaylist();

                if (ui->timerToneSelect->currentText() == tr("Happy Bee")) {
                    playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/happybee")));
                } else if (ui->timerToneSelect->currentText() == tr("Playing in the Dark")) {
                    playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/playinginthedark")));
                } else if (ui->timerToneSelect->currentText() == tr("Ice Cream Truck")) {
                    playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/icecream")));
                } else if (ui->timerToneSelect->currentText() == tr("Party Complex")) {
                    playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/party")));
                } else if (ui->timerToneSelect->currentText() == tr("Salty Ditty")) {
                    playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/saltyditty")));
                }
                playlist->setPlaybackMode(QMediaPlaylist::Loop);
                ringtone->setPlaylist(playlist);
                ringtone->play();

                AudioMan->attenuateStreams();
            }
            updateTimers();
        } else {
            ui->label_7->setText(timeUntilTimeout.toString("HH:mm:ss"));
            updateTimers();
        }
    });
    timer->start();
    updateTimers();
}

void InfoPaneDropdown::updateTimers() {
    QStringList parts;
    if (timer != NULL) {
        parts.append(timeUntilTimeout.toString("HH:mm:ss"));
    }

    if (stopwatchRunning) {
        QTime stopwatchTime = QTime::fromMSecsSinceStartOfDay(0);
        stopwatchTime = stopwatchTime.addMSecs(stopwatchTimeAdd);
        stopwatchTime = stopwatchTime.addMSecs(this->stopwatchTime.elapsed());
        parts.append(stopwatchTime.toString("hh:mm:ss"));
    }

    if (parts.count() != 0) {
        emit timerVisibleChanged(true);
        emit timerChanged(parts.join(" · "));
    } else {
        emit timerVisibleChanged(false);
    }
}

void InfoPaneDropdown::notificationClosed(uint id, uint reason) {
    Q_UNUSED(reason)
    if (id == timerNotificationId) {
        ringtone->stop();
        AudioMan->restoreStreams();
        timerNotificationId = 0;
    }
}

void InfoPaneDropdown::on_pushButton_2_clicked()
{
    if (timer == NULL) {
        startTimer(ui->timeEdit->time());
    } else {
        if (timer->isActive()) {
            timer->stop();
            ui->pushButton_3->setVisible(true);
            ui->label_7->setEnabled(false);
            ui->pushButton_2->setText(tr("Resume"));
        } else {
            timer->start();
            ui->pushButton_3->setVisible(false);
            ui->label_7->setEnabled(true);
            ui->pushButton_2->setText(tr("Pause"));
        }
    }
}

void InfoPaneDropdown::on_pushButton_3_clicked()
{
    delete timer;
    timer = NULL;
    ui->timeEdit->setVisible(true);
    ui->label_7->setVisible(false);
    ui->label_7->setEnabled(true);
    ui->pushButton_2->setText(tr("Start"));
    ui->pushButton_3->setVisible(false);
    emit timerVisibleChanged(false);
    emit timerEnabledChanged(true);
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

void InfoPaneDropdown::on_lineEdit_2_editingFinished()
{
    //settings.setValue("startup/autostart", ui->lineEdit_2->text());
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
    QProcess::startDetached("redshift -O " + QString::number(position));
}

void InfoPaneDropdown::on_redshiftIntensity_sliderReleased()
{
    if (!isRedshiftOn) {
        QProcess::startDetached("redshift -O 6500");
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
        //ui->clearAllNotifications->setVisible(true);
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
        //ui->clearAllNotifications->setVisible(false);
    }
}

void InfoPaneDropdown::on_clearAllNotifications_clicked()
{
    for (int id : notificationFrames.keys()) {
        emit closeNotification(id);
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
            if (days == 1) {
                uptimeString = tr("1 day") + " " + sysUptime.toString("hh:mm:ss");
            } else {
                uptimeString = tr("%1 days", NULL, days).arg(days) + " " + sysUptime.toString("hh:mm:ss");
            }
        } else {
            uptimeString = sysUptime.toString("hh:mm:ss");
        }
        ui->systemUptime->setText(tr("System Uptime: %1").arg(uptimeString));
    } else {
        ui->systemUptime->setText(tr("Couldn't get system uptime"));
    }
    delete info;
}

void InfoPaneDropdown::on_printLabel_clicked()
{
    //changeDropDown(Print);
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

bool InfoPaneDropdown::isTimerRunning() {
    if (timer == NULL) {
        return false;
    } else {
        return true;
    }
}

void InfoPaneDropdown::mousePressEvent(QMouseEvent *event) {
    mouseClickPoint = event->localPos().toPoint().y();
    initialPoint = mouseClickPoint;
    dragRect = this->geometry();
    mouseMovedUp = false;
    event->accept();
}

void InfoPaneDropdown::mouseMoveEvent(QMouseEvent *event) {
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
        if (dragRect.top() <= screenGeometry.top()) {
            dragRect.moveTo(screenGeometry.left(), screenGeometry.top());
        }
    }
    this->setGeometry(dragRect);

    mouseClickPoint = event->globalY();
    event->accept();
}

void InfoPaneDropdown::mouseReleaseEvent(QMouseEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (initialPoint - 5 > mouseClickPoint && initialPoint + 5 < mouseClickPoint) {
        tPropertyAnimation* a = new tPropertyAnimation(this, "geometry");
        a->setStartValue(this->geometry());
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
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
            a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
            a->setEasingCurve(QEasingCurve::OutCubic);
            a->setDuration(500);
            connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
            a->start();
        }
    }
    event->accept();
    initialPoint = 0;
}

void InfoPaneDropdown::on_TouchFeedbackSwitch_toggled(bool checked)
{
    settings.setValue("input/touchFeedbackSound", checked);
}

void InfoPaneDropdown::on_thewaveTTSpico2wave_clicked()
{
    settings.setValue("thewave/ttsEngine", "pico2wave");
}

void InfoPaneDropdown::on_thewaveTTSfestival_clicked()
{
    settings.setValue("thewave/ttsEngine", "festival");
}


void InfoPaneDropdown::on_thewaveWikipediaSwitch_toggled(bool checked)
{
    settings.setValue("thewave/wikipediaSearch", checked);
}

void InfoPaneDropdown::on_thewaveTTSespeak_clicked()
{
    settings.setValue("thewave/ttsEngine", "espeak");
}

void InfoPaneDropdown::on_thewaveOffensiveSwitch_toggled(bool checked)
{
    settings.setValue("thewave/blockOffensiveWords", checked);
}

void InfoPaneDropdown::on_theWaveName_textEdited(const QString &arg1)
{
    settings.setValue("thewave/name", arg1);
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
    if (currentRow == 9) { //Users
        setupUsersSettingsPane();
    } else if (currentRow == 10) { //Date and Time
        setupDateTimeSettingsPane();
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

void InfoPaneDropdown::on_thewaveTTSsilent_clicked()
{
    settings.setValue("thewave/ttsEngine", "none");
}

void InfoPaneDropdown::on_theWaveSwitch_toggled(bool checked)
{
    settings.setValue("thewave/enabled", checked);
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

void InfoPaneDropdown::updateKdeconnect() {
    if (!QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.kde.kdeconnect")) {
        ui->kdeconnectNotRunning->setVisible(true);
        ui->kdeconnectArea->setVisible(false);
    } else {
        ui->kdeconnectNotRunning->setVisible(false);
        ui->kdeconnectArea->setVisible(true);
        int currentSelectedDevice = ui->kdeconnectDevices->currentRow();
        QDBusInterface kdeconnectDaemon("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon");
        QDBusReply<QStringList> foundDevices = kdeconnectDaemon.call("devices", false, true);

        ui->kdeconnectDevices->clear();
        for (QString device : foundDevices.value()) {
            QDBusInterface deviceInterface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device, "org.kde.kdeconnect.device");
            bool isReachable = deviceInterface.property("isReachable").toBool();
            QListWidgetItem* item = new QListWidgetItem;
            item->setText(deviceInterface.property("name").toString());
            item->setIcon(QIcon::fromTheme(deviceInterface.property("iconName").toString()));
            item->setData(Qt::UserRole, device);
            if (!isReachable) {
                item->setForeground(ui->kdeconnectDevices->palette().brush(QPalette::Disabled, QPalette::Text));
            }
            ui->kdeconnectDevices->addItem(item);
        }

        if (currentSelectedDevice != -1 && ui->kdeconnectDevices->count() > currentSelectedDevice) {
            ui->kdeconnectDevices->setCurrentRow(currentSelectedDevice);
        }
    }
}

void InfoPaneDropdown::on_startKdeconnect_clicked()
{
    //Start KDE Connect
    QProcess::startDetached("/usr/lib/kdeconnectd");
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

    if (switchingWidget == ui->clockFrame) {
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

void InfoPaneDropdown::on_stopwatchStart_clicked()
{
    if (stopwatchRunning) {
        stopwatchRunning = false;
        stopwatchTimeAdd += stopwatchTime.elapsed();

        ui->stopwatchReset->setVisible(true);
        ui->stopwatchStart->setText(tr("Start"));
        ui->stopwatchStart->setIcon(QIcon::fromTheme("media-playback-start"));
    } else {
        stopwatchTime.restart();
        stopwatchRunning = true;

        ui->stopwatchReset->setVisible(false);
        ui->stopwatchStart->setText(tr("Stop"));
        ui->stopwatchStart->setIcon(QIcon::fromTheme("media-playback-stop"));
    }
}

void InfoPaneDropdown::on_stopwatchReset_clicked()
{
    stopwatchTimeAdd = 0;
}

void InfoPaneDropdown::on_calendarTodayButton_clicked()
{
    ui->calendarWidget->setSelectedDate(QDate::currentDate());
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
    }
}

void InfoPaneDropdown::on_darkColorThemeRadio_toggled(bool checked)
{
    if (checked) {
        themeSettings->setValue("color/type", "dark");
        updateAccentColourBox();
        resetStyle();
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
    //ui->systemFont->setFont(QFont(themeSettings->value("font/defaultFamily", defaultFont).toString(), themeSettings->value("font/defaultSize", 10).toInt()));
}

void InfoPaneDropdown::on_locateDeviceButton_clicked()
{
    if (ui->kdeconnectDevices->selectedItems().count() != 0) {
        if (QMessageBox::question(this, tr("Locate Device"), tr("Your device will ring at full volume. Tap the button on the screen of the device to silence it."), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok) {
            QString device = ui->kdeconnectDevices->selectedItems().first()->data(Qt::UserRole).toString();
            QDBusInterface findPhone("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device + "/findmyphone", "org.kde.kdeconnect.device.findmyphone");
            findPhone.call("ring");
        }
    }
}

void InfoPaneDropdown::on_pingDeviceButton_clicked()
{
    if (ui->kdeconnectDevices->selectedItems().count() != 0) {
        QString device = ui->kdeconnectDevices->selectedItems().first()->data(Qt::UserRole).toString();
        QDBusInterface findPhone("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + device + "/ping", "org.kde.kdeconnect.device.ping");
        findPhone.call("sendPing");
    }
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
        if (historyArgument.isValid()) {
            QDBusArgument arrayArgument = historyArgument.value();
            arrayArgument.beginArray();
            while (!arrayArgument.atEnd()) {
                BatteryInfo info;
                arrayArgument >> info;

                qint64 msecs = info.time;
                msecs = msecs * 1000;

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
            xAxis->setMin(xAxis->max().addDays(-1));
        } else {
            xAxis->setMax(QDateTime::currentDateTime());
            xAxis->setMin(xAxis->max().addSecs(-43200)); //Half a day
        }
        batteryChart->addAxis(xAxis, Qt::AlignBottom);
        xAxis->setLabelsColor(this->palette().color(QPalette::WindowText));
        xAxis->setFormat("dd/MM/yy hh:mm");
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
        this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height() + y, screenGeometry.width(), screenGeometry.height());
    } else {
        this->setGeometry(screenGeometry.x(), screenGeometry.top() + y, screenGeometry.width(), screenGeometry.height());
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
    this->setFixedHeight(screenGeometry.height());

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
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), screenGeometry.height()));
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

    QDBusMessage launchMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "StartServiceByName");
    QVariantList args;
    args.append("org.freedesktop.timedate1");
    args.append((uint) 0);
    launchMessage.setArguments(args);

    QDBusConnection::systemBus().call(launchMessage);
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

RemindersListModel::RemindersListModel(QObject *parent) : QAbstractListModel(parent) {
    RemindersData = new QSettings("theSuite/theShell.reminders");
    RemindersData->beginGroup("reminders");
}

RemindersListModel::~RemindersListModel() {
    RemindersData->endGroup();
    RemindersData->deleteLater();
}

int RemindersListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    int count = RemindersData->beginReadArray("reminders");
    RemindersData->endArray();
    return count;
}

QVariant RemindersListModel::data(const QModelIndex &index, int role) const {
    QVariant returnValue;

    RemindersData->beginReadArray("reminders");
    RemindersData->setArrayIndex(index.row());
    if (role == Qt::DisplayRole) {
        returnValue = RemindersData->value("title");
    } else if (role == Qt::UserRole) {
        QDateTime activation = RemindersData->value("date").toDateTime();
        if (activation.daysTo(QDateTime::currentDateTime()) == 0) {
            returnValue = activation.toString("hh:mm");
        } else if (activation.daysTo(QDateTime::currentDateTime()) < 7) {
            returnValue = activation.toString("dddd");
        } else {
            returnValue = activation.toString("ddd, dd MMM yyyy");
        }
    }

    RemindersData->endArray();
    return returnValue;
}

void RemindersListModel::updateData() {
    emit dataChanged(index(0), index(rowCount()));
}

RemindersDelegate::RemindersDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void RemindersDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);

    QRect textRect;
    textRect.setLeft(6 * getDPIScaling());
    textRect.setTop(option.rect.top() + 6 * getDPIScaling());
    textRect.setBottom(option.rect.top() + option.fontMetrics.height() + 6 * getDPIScaling());
    textRect.setRight(option.rect.right());

    QRect dateRect;
    dateRect.setLeft(6 * getDPIScaling());
    dateRect.setTop(option.rect.top() + option.fontMetrics.height() + 8 * getDPIScaling());
    dateRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + 6 * getDPIScaling());
    dateRect.setRight(option.rect.right());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
        painter->drawText(textRect, index.data().toString());
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawText(textRect, index.data().toString());
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
        painter->drawText(dateRect, index.data(Qt::UserRole).toString());
    }
}

QSize RemindersDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.fontMetrics.width(index.data().toString()), option.fontMetrics.height() * 2 + 14 * getDPIScaling());
}

void InfoPaneDropdown::on_ReminderCancel_clicked()
{
    ui->RemindersStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_ReminderNew_clicked()
{
    ui->ReminderTitle->setText("");
    ui->ReminderDate->setDateTime(QDateTime::currentDateTime().addSecs(3600)); //Current date + 1 hour
    ui->RemindersStackedWidget->setCurrentIndex(1);
}

void InfoPaneDropdown::on_ReminderCreate_clicked()
{
    if (ui->ReminderTitle->text() == "") {
        return;
    }

    QList<QPair<QString, QDateTime>> ReminderData;

    QSettings reminders("theSuite/theShell.reminders");
    reminders.beginGroup("reminders");
    int count = reminders.beginReadArray("reminders");

    for (int i = 0; i < count; i++) {
        reminders.setArrayIndex(i);
        QPair<QString, QDateTime> data;
        data.first = reminders.value("title").toString();
        data.second = reminders.value("date").toDateTime();
        ReminderData.append(data);
    }

    QPair<QString, QDateTime> newData;
    newData.first = ui->ReminderTitle->text();
    newData.second = ui->ReminderDate->dateTime().addSecs(-ui->ReminderDate->dateTime().time().second());
    ReminderData.append(newData);

    reminders.endArray();
    reminders.beginWriteArray("reminders");
    int i = 0;
    for (QPair<QString, QDateTime> data : ReminderData) {
        reminders.setArrayIndex(i);
        reminders.setValue("title", data.first);
        reminders.setValue("date", data.second);
        i++;
    }
    reminders.endArray();
    reminders.endGroup();

    ((RemindersListModel*) ui->RemindersList->model())->updateData();
    ui->RemindersStackedWidget->setCurrentIndex(0);
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
    //Set flags that persist between changes
    settings.setValue("flightmode/on", checked);
    if (checked) {
        settings.setValue("flightmode/wifi", ui->WifiSwitch->isChecked());
        settings.setValue("flightmode/bt", ui->BluetoothSwitch->isChecked());

        //Disable bluetooth and WiFi.
        ui->WifiSwitch->setChecked(false);
        ui->BluetoothSwitch->setChecked(false);
    } else {
        //Enable bluetooth and WiFi.
        ui->WifiSwitch->setChecked(settings.value("flightmode/wifi", true).toBool());
        ui->BluetoothSwitch->setChecked(settings.value("flightmode/bt", true).toBool());
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

void InfoPaneDropdown::on_ReminderDelete_clicked()
{
    if (ui->RemindersList->selectionModel()->selectedRows().count() != 0) {
        //Show delete screen
        ui->RemindersStackedWidget->setCurrentIndex(2);

        ui->deleteReminderText->setText(ui->RemindersList->model()->data(ui->RemindersList->selectionModel()->selectedRows().first()).toString());
        ui->deleteReminderTime->setText(ui->RemindersList->model()->data(ui->RemindersList->selectionModel()->selectedRows().first(), Qt::UserRole).toString());
    }
}

void InfoPaneDropdown::on_RemindersStackedWidget_currentChanged(int arg1)
{
    tVariantAnimation* height = new tVariantAnimation();
    height->setDuration(250);
    height->setEasingCurve(QEasingCurve::InOutCubic);
    height->setStartValue(ui->RemindersStackedWidget->height());
    if (arg1 == 0) {
        height->setEndValue((int) (300 * getDPIScaling() - 10));
        //ui->ClockScrollArea->verticalScrollBar()->setEnabled(true);
        ui->ClockScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        height->setEndValue(ui->ClockScrollArea->height() - 10);
        //ui->ClockScrollArea->verticalScrollBar()->setEnabled(false);
        ui->ClockScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    connect(height, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->RemindersStackedWidget->setFixedHeight(value.toInt());
        ui->ClockScrollArea->verticalScrollBar()->setValue(ui->ClockScrollArea->verticalScrollBar()->maximum());
    });
    connect(height, &tVariantAnimation::finished, [=] {
        ui->ClockScrollArea->verticalScrollBar()->setValue(ui->ClockScrollArea->verticalScrollBar()->maximum());
    });
    connect(height, SIGNAL(finished()), height, SLOT(deleteLater()));
    height->start();
}

void InfoPaneDropdown::on_ReminderDeleteCancel_clicked()
{
    //Go back to normal screen
    ui->RemindersStackedWidget->setCurrentIndex(0);
}

void InfoPaneDropdown::on_ReminderDeleteButton_clicked()
{
    QList<QPair<QString, QDateTime>> ReminderData;

    QSettings reminders("theSuite/theShell.reminders");
    reminders.beginGroup("reminders");
    int count = reminders.beginReadArray("reminders");

    for (int i = 0; i < count; i++) {
        reminders.setArrayIndex(i);
        QPair<QString, QDateTime> data;
        data.first = reminders.value("title").toString();
        data.second = reminders.value("date").toDateTime();
        ReminderData.append(data);
    }

    reminders.endArray();

    ReminderData.removeAt(ui->RemindersList->selectionModel()->selectedIndexes().at(0).row());

    reminders.beginWriteArray("reminders");
    int i = 0;
    for (QPair<QString, QDateTime> data : ReminderData) {
        reminders.setArrayIndex(i);
        reminders.setValue("title", data.first);
        reminders.setValue("date", data.second);
        i++;
    }
    reminders.endArray();
    reminders.endGroup();

    ((RemindersListModel*) ui->RemindersList->model())->updateData();
    ui->RemindersStackedWidget->setCurrentIndex(0);
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
        ui->upArrow->setPixmap(QIcon::fromTheme("go-up").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    } else {
        ((QBoxLayout*) this->layout())->setDirection(QBoxLayout::BottomToTop);
        ((QBoxLayout*) ui->partFrame->layout())->setDirection(QBoxLayout::BottomToTop);
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
        sessionSettings->setValue("screen/dpi", 96);
    }
}

void InfoPaneDropdown::on_dpi150_toggled(bool checked)
{
    if (checked) {
        sessionSettings->setValue("screen/dpi", 144);
    }
}

void InfoPaneDropdown::on_dpi200_toggled(bool checked)
{
    if (checked) {
        sessionSettings->setValue("screen/dpi", 192);
    }
}

void InfoPaneDropdown::on_dpi300_toggled(bool checked)
{
    if (checked) {
        sessionSettings->setValue("screen/dpi", 288);
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

void InfoPaneDropdown::notificationAction(uint id, QString action) {
    if (id == timerNotificationId) {
        //Preserve old timer in case user wants to restart it
        QTime lastTimer = this->lastTimer;

        ringtone->stop();
        AudioMan->restoreStreams();
        timerNotificationId = 0;

        if (action == "+0.5") {
            startTimer(QTime(0, 0, 30));
        } else if (action == "+1") {
            startTimer(QTime(0, 1));
        } else if (action == "+2") {
            startTimer(QTime(0, 2));
        } else if (action == "+5") {
            startTimer(QTime(0, 5));
        } else if (action == "+10") {
            startTimer(QTime(0, 10));
        } else if (action == "restart") {
            startTimer(lastTimer);
        }
        this->lastTimer = lastTimer;
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

    ui->startupStack->setCurrentIndex(2);
}

void InfoPaneDropdown::on_enterCommandAutoStartApps_clicked()
{
    ui->autostartAppName->setText("");
    ui->autostartAppCommand->setText("");
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
    }
}
