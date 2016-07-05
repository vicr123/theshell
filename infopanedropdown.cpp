#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void EndSession(EndSessionWait::shutdownType type);

InfoPaneDropdown::InfoPaneDropdown(NotificationDBus* notificationEngine, UPowerDBus* powerEngine, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    ui->setupUi(this);

    this->notificationEngine = notificationEngine;
    this->powerEngine = powerEngine;
    notificationEngine->setDropdownPane(this);

    connect(notificationEngine, SIGNAL(newNotification(int,QString,QString,QIcon)), this, SLOT(newNotificationReceived(int,QString,QString,QIcon)));
    connect(notificationEngine, SIGNAL(removeNotification(int)), this, SLOT(removeNotification(int)));
    connect(notificationEngine, SIGNAL(NotificationClosed(int,int)), this, SLOT(notificationClosed(int,int)));
    connect(this, SIGNAL(closeNotification(int)), notificationEngine, SLOT(CloseNotificationUserInitiated(int)));

    connect(powerEngine, SIGNAL(batteryChanged(int)), this, SLOT(batteryLevelChanged(int)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    timer->setInterval(1000);
    timer->start();

    ui->label_7->setVisible(false);
    ui->pushButton_3->setVisible(false);
    ui->networkKey->setVisible(false);
    ui->networkConnect->setVisible(false);

    {
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "DeviceAdded", this, SLOT(newNetworkDevice(QDBusObjectPath)));
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "DeviceRemoved", this, SLOT(getNetworks()));

        QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
        QDBusReply<QList<QDBusObjectPath>> reply = i->call("GetDevices");

        if (reply.isValid()) {
            for (QDBusObjectPath device : reply.value()) {
                newNetworkDevice(device);
            }
        }

        delete i;
    }

    connect(this, &InfoPaneDropdown::networkLabelChanged, [=](QString label) {
        ui->networkStatus->setText(label);
    });

    ui->FlightSwitch->setOnIcon(getIconFromTheme("flight.svg", this->palette().color(QPalette::Window)));

    if (!QFile("/usr/bin/systemsettings5").exists()) {
        ui->pushButton_8->setVisible(false);
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

    ui->lockScreenBackground->setText(lockScreenSettings->value("background", "/usr/share/icons/theos/backgrounds/triangle/1920x1080.png").toString());
    ui->lineEdit_2->setText(settings.value("startup/autostart", "").toString());
    ui->redshiftPause->setChecked(!settings.value("display/redshiftPaused", true).toBool());
    ui->TouchFeedbackSwitch->setChecked(settings.value("input/touchFeedbackSound", false).toBool());
    ui->thewaveWikipediaSwitch->setChecked(settings.value("thewave/wikipediaSearch", true).toBool());
    ui->thewaveOffensiveSwitch->setChecked(settings.value("thewave/blockOffensiveWords", true).toBool());
    ui->theWaveName->setText(settings.value("thewave/name", "").toString());
    ui->TextSwitch->setChecked(settings.value("bar/showText", true).toBool());
    ui->windowManager->setText(settings.value("startup/WindowManagerCommand", "kwin_x11").toString());
    ui->barDesktopsSwitch->setChecked(settings.value("bar/showWindowsFromOtherDesktops", true).toBool());

    eventTimer = new QTimer(this);
    eventTimer->setInterval(1000);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(processTimer()));
    eventTimer->start();

    UGlobalHotkeys* exitKey = new UGlobalHotkeys(this);
    exitKey->registerHotkey("Alt+F7");
    connect(exitKey, SIGNAL(activated(size_t)), this, SLOT(on_pushButton_clicked()));

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

    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("preferences-system-login"), "Startup"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("preferences-desktop"), "Bar"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("preferences-desktop-display"), "Display"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-warning"), "Notifications"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("input-tablet"), "Input"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("system-lock-screen"), "Lock Screen"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("thewave", QIcon(":/icons/thewave.svg")), "theWave"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("emblem-warning"), "Danger"));
    ui->settingsList->addItem(new QListWidgetItem(QIcon::fromTheme("help-about"), "About"));
    ui->settingsList->item(ui->settingsList->count() - 1)->setSelected(true);

    ringtone = new QMediaPlayer(this, QMediaPlayer::LowLatency);
    ui->timerToneSelect->addItem("Happy Bee");
    ui->timerToneSelect->addItem("Playing in the Dark");
}

InfoPaneDropdown::~InfoPaneDropdown()
{
    delete ui;
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
    getNetworks();
    i->deleteLater();
}

void InfoPaneDropdown::on_WifiSwitch_toggled(bool checked)
{
    QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
    if (i->property("WirelessEnabled").toBool() != checked) {
        i->setProperty("WirelessEnabled", checked);
    }
    i->deleteLater();
    if (!ui->WifiSwitch->isChecked()) {
        ui->WifiSwitch->setChecked(checked);
    }
}

void InfoPaneDropdown::processTimer() {
    QTime time = QTime::currentTime();
    if (ui->redshiftPause->isChecked() && time.secsTo(ui->startRedshift->time()) <= 0 && time.secsTo(ui->endRedshift->time()) >= 0) {
        if (!isRedshiftOn) {
            isRedshiftOn = true;
            QProcess::startDetached("redshift -O " + QString::number(ui->redshiftIntensity->value()));
        }
    } else {
        if (isRedshiftOn) {
            isRedshiftOn = false;
            QProcess::startDetached("redshift -O 6500");
        }
    }

    {
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
                QPixmap icon = QIcon::fromTheme("printer").pixmap(22, 22);
                if (currentDestination.is_default) {
                    QPainter *p = new QPainter();
                    p->begin(&icon);
                    p->drawPixmap(10, 10, 12, 12, QIcon::fromTheme("emblem-checked").pixmap(12, 12));
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
                statLabel->setText("Idle");
                statLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                statLayout->addWidget(statLabel);

                /*QPushButton* button = new QPushButton();
                button->setIcon(QIcon::fromTheme("window-close"));
                connect(button, &QPushButton::clicked, [=]() {
                    emit closeNotification(id);
                });
                layout->addWidget(button);*/

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
                        state = "Idle";
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "4", 1) == 0) {
                        state = "Printing";
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "5", 1) == 0) {
                        state = "Stopped";
                        printersStatFrames.value(currentDestination.name)->setEnabled(false);
                    }
                } else if (strncmp(currentOption.name, "printer-state-reasons", strlen(currentOption.name)) == 0) {
                    stateReasons = QString::fromUtf8(currentOption.value, strlen(currentOption.value));
                }
            }
            printersStats.value(currentDestination.name)->setText(state + " / " + stateReasons);

        }

        cupsFreeDests(destinationCount, destinations);
    }
}

void InfoPaneDropdown::timerTick() {
    ui->date->setText(QDateTime::currentDateTime().toString("ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

}

void InfoPaneDropdown::show(dropdownType showWith) {
    changeDropDown(showWith);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), screenGeometry.width(), screenGeometry.height());

    unsigned long desktop = 0xFFFFFFFF;
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();

    QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());
    a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();

    //Get Current Brightness
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -get");
    backlight->waitForFinished();
    float output = ceil(QString(backlight->readAll()).toFloat());
    delete backlight;

    ui->brightnessSlider->setValue((int) output);
}

void InfoPaneDropdown::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());
    a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), this->width(), this->height()));
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, &QPropertyAnimation::finished, [=]() {
        QDialog::close();
    });
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();
}

void InfoPaneDropdown::changeDropDown(dropdownType changeTo) {
    //QFrame *currentDropDownFrame;
    this->currentDropDown = changeTo;

    ui->networkFrame->setVisible(false);
    ui->batteryFrame->setVisible(false);
    ui->notificationsFrame->setVisible(false);
    ui->clockFrame->setVisible(false);
    ui->settingsFrame->setVisible(false);
    ui->printFrame->setVisible(false);

    ui->clockLabel->setShowDisabled(true);
    ui->batteryLabel->setShowDisabled(true);
    ui->notificationsLabel->setShowDisabled(true);
    ui->networkLabel->setShowDisabled(true);
    ui->printLabel->setShowDisabled(true);

    switch (changeTo) {
    case Settings:
        ui->settingsFrame->setVisible(true);

        break;
    case Clock:
        ui->clockFrame->setVisible(true);
        ui->clockLabel->setShowDisabled(false);
        break;
    case Battery:
        ui->batteryFrame->setVisible(true);
        ui->batteryLabel->setShowDisabled(false);
        break;
    case Notifications:
        ui->notificationsFrame->setVisible(true);
        ui->notificationsLabel->setShowDisabled(false);
        break;
    case Network:
        ui->networkFrame->setVisible(true);
        ui->networkLabel->setShowDisabled(false);

        getNetworks();
        break;
    case Print:
        ui->printFrame->setVisible(true);
        ui->printLabel->setShowDisabled(false);

    }

    if (changeTo == 0) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(true);
    } else if (changeTo == 4) {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(false);
    } else if (changeTo == -1) {
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
    QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);

    QDBusReply<QList<QDBusObjectPath>> reply = i->call("GetDevices");

    QString NetworkLabel = "Disconnected from the Internet";
    if (ui->FlightSwitch->isChecked()) {
        NetworkLabel = "Flight Mode";
    }
    enum NetworkType {
        None = 0,
        Bluetooth = 1,
        Wireless = 2,
        Wired = 3
    };

    NetworkType NetworkLabelType = NetworkType::None;

    if (reply.isValid()) {
        ui->networkList->clear();
        for (QDBusObjectPath device : reply.value()) {
            QDBusInterface *deviceInterface = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus(), this);
            QString interface = deviceInterface->property("Interface").toString();
            switch (deviceInterface->property("DeviceType").toInt()) {
            case 1: //Ethernet
            {
                QDBusInterface *wire = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wired", QDBusConnection::systemBus(), this);
                if (wire->property("Carrier").toBool()) { //Connected to a network
                    NetworkLabel = "Connected over a wired connection";
                    NetworkLabelType = NetworkType::Wired;
                }
                delete wire;
            }
                break;
            case 2: //WiFi
            {
                QDBusInterface *wifi = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus(), this);
                QString connectedSsid;
                { //Detect Connected Network
                    if (NetworkLabelType < NetworkType::Wireless) {
                        QDBusInterface *ap = new QDBusInterface("org.freedesktop.NetworkManager", wifi->property("ActiveAccessPoint").value<QDBusObjectPath>().path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus(), this);
                        switch (deviceInterface->property("State").toInt()) {
                        case 30:
                            if (!connectedNetworks.keys().contains(interface)) {
                                connectedNetworks.insert(interface, "");
                            } else {
                                if (connectedNetworks.value(interface) != "") {
                                    connectedNetworks.insert(interface, "");
                                    QVariantMap hints;
                                    hints.insert("category", "network.disconnected");
                                    hints.insert("transient", true);
                                    notificationEngine->Notify("theShell", 0, "", "Wireless Connection",
                                                               "You've been disconnected from the internet",
                                                               QStringList(), hints, -1);
                                }
                            }
                            break;
                        case 40:
                        case 50:
                        case 60:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Connecting to " + connectedSsid + "...";
                            NetworkLabelType = NetworkType::Wireless;
                            break;
                        case 70:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Getting IP Address from " + connectedSsid + "...";
                            NetworkLabelType = NetworkType::Wireless;
                            break;
                        case 80:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Doing some checks...";
                            NetworkLabelType = NetworkType::Wireless;
                            break;
                        case 90:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Connecting to a secondary connection...";
                            NetworkLabelType = NetworkType::Wireless;
                            break;
                        case 100:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Connected to " + connectedSsid;
                            NetworkLabelType = NetworkType::Wireless;
                            ui->networkMac->setText("MAC Address: " + wifi->property("PermHwAddress").toString());
                            if (!connectedNetworks.keys().contains(interface)) {
                                connectedNetworks.insert(interface, connectedSsid);
                            } else {
                                if (connectedNetworks.value(interface) != connectedSsid) {
                                    connectedNetworks.insert(interface, connectedSsid);
                                    QVariantMap hints;
                                    hints.insert("category", "network.connected");
                                    hints.insert("transient", true);
                                    notificationEngine->Notify("theShell", 0, "", "Wireless Connection",
                                                               "You're now connected to the network \"" + connectedSsid + "\"",
                                                               QStringList(), hints, -1);
                                }
                            }
                            break;
                        case 110:
                        case 120:
                            connectedSsid = ap->property("Ssid").toString();
                            NetworkLabel = "Disconnecting from " + connectedSsid + "...";
                            NetworkLabelType = NetworkType::Wireless;
                            break;
                        }
                        delete ap;
                    }
                }

                { //Detect Available Networks
                    QList<QDBusObjectPath> accessPoints = wifi->property("AccessPoints").value<QList<QDBusObjectPath>>();
                    for (QDBusObjectPath accessPoint : accessPoints) {
                        QDBusInterface *ap = new QDBusInterface("org.freedesktop.NetworkManager", accessPoint.path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus(), this);

                        uchar strength = ap->property("Strength").value<uchar>();

                        QListWidgetItem* apItem = new QListWidgetItem();
                        apItem->setText(ap->property("Ssid").toString());
                        if (strength < 15) {
                            apItem->setIcon(QIcon::fromTheme("network-wireless-connected-00"));
                        } else if (strength < 35) {
                            apItem->setIcon(QIcon::fromTheme("network-wireless-connected-25"));
                        } else if (strength < 65) {
                            apItem->setIcon(QIcon::fromTheme("network-wireless-connected-50"));
                        } else if (strength < 85) {
                            apItem->setIcon(QIcon::fromTheme("network-wireless-connected-75"));
                        } else {
                            apItem->setIcon(QIcon::fromTheme("network-wireless-connected-100"));
                        }
                        if (ap->property("Ssid") == connectedSsid) {
                            apItem->setBackground(QBrush(QColor(0, 255, 0, 100)));
                        }
                        apItem->setData(Qt::UserRole, QVariant::fromValue(accessPoint));
                        apItem->setData(Qt::UserRole + 1, QVariant::fromValue(device));
                        apItem->setData(Qt::UserRole + 2, ap->property("Ssid") == connectedSsid);
                        ui->networkList->addItem(apItem);

                        delete ap;
                    }
                }

                delete wifi;
            }

                break;
            case 5: //Bluetooth
            {
                if (NetworkLabelType < NetworkType::Bluetooth) {
                    QDBusInterface *bt = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Bluetooth", QDBusConnection::systemBus(), this);
                    switch (deviceInterface->property("State").toInt()) {
                        case 100:
                        NetworkLabel = "Connected to " + bt->property("Name").toString() + " over Bluetooth";
                        NetworkLabelType = NetworkType::Bluetooth;
                    }
                }
            }
                break;
            }
            delete deviceInterface;
        }
    } else {
        NetworkLabel = "NetworkManager Error";
    }

    {
        //QDBusObjectPath active = i->property("PrimaryConnection").value<QDBusObjectPath>();
        //if (active.path() == "/") {
            ui->networkInfoFrame->setVisible(false);
        /*} else {
            QDBusInterface *conn = new QDBusInterface("org.freedesktop.NetworkManager", active.path(), "org.freedesktop.NetworkManager.Connection.Active", QDBusConnection::systemBus(), this);
            {
                QDBusObjectPath ipv4 = conn->property("Ip4Config").value<QDBusObjectPath>();
                //QDBusInterface *ip4 = new QDBusInterface("org.freedesktop.NetworkManager", ipv4.path(), "org.freedesktop.NetworkManager.IP4Config", QDBusConnection::systemBus(), this);
                QDBusInterface ip4("org.freedesktop.NetworkManager", ipv4.path(), "org.freedesktop.NetworkManager.IP4Config", QDBusConnection::systemBus(), this);

                QList<QVariantMap> addressData = ip4.property("AddressData").value<QList<QVariantMap>>();
                ui->networkIpv4->setText("IPv4 Address: " + addressData.first().value("address").toString());
                //delete ip4;
            }
            {
                QDBusObjectPath ipv6 = conn->property("Ip6Config").value<QDBusObjectPath>();
                QDBusInterface *ip6 = new QDBusInterface("org.freedesktop.NetworkManager", ipv6.path(), "org.freedesktop.NetworkManager.IP6Config", QDBusConnection::systemBus(), this);
                QList<QVariantMap> addressData = ip6->property("AddressData").value<QList<QVariantMap>>();
                ui->networkIpv6->setText("IPv6 Address: " + addressData.first().value("address").toString());
                delete ip6;
            }
            ui->networkInfoFrame->setVisible(true);
        }*/
    }

    delete i;
    emit networkLabelChanged(NetworkLabel);
}

void InfoPaneDropdown::on_networkList_currentItemChanged(QListWidgetItem *current, QListWidgetItem*)
{
    ui->networkKey->setText("");
    if (current == NULL || !current) {
        ui->networkKey->setVisible(false);
        ui->networkConnect->setVisible(false);
    } else {
        if (current->data(Qt::UserRole + 2).toBool()) { //Connected to this network
            ui->networkKey->setVisible(false);
            ui->networkConnect->setText("Disconnect");
            ui->networkConnect->setIcon(QIcon::fromTheme("network-disconnect"));
            ui->networkConnect->setVisible(true);
        } else { //Not connected to this network
            QDBusInterface *ap = new QDBusInterface("org.freedesktop.NetworkManager", current->data(Qt::UserRole).value<QDBusObjectPath>().path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus(), this);

            bool isSaved = false;
            QDBusInterface *settings = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus(), this);
            QDBusReply<QList<QDBusObjectPath>> allConnections = settings->call("ListConnections");
            for (QDBusObjectPath connection : allConnections.value()) {
                QDBusInterface *settings = new QDBusInterface("org.freedesktop.NetworkManager", connection.path(), "org.freedesktop.NetworkManager.Settings.Connection", QDBusConnection::systemBus(), this);

                QDBusReply<QMap<QString, QVariantMap>> reply = settings->call("GetSettings");
                QMap<QString, QVariantMap> connectionSettings = reply.value();
                if (connectionSettings.value("802-11-wireless").value("ssid").toString() == ap->property("Ssid")) {
                    isSaved = true;
                }
            }

            current->setData(Qt::UserRole + 3, isSaved);

            if (ap->property("WpaFlags").toUInt() != 0 && !isSaved) {
                ui->networkKey->setVisible(true);
            } else {
                ui->networkKey->setVisible(false);
            }
            ui->networkConnect->setText("Connect");
            ui->networkConnect->setIcon(QIcon::fromTheme("network-connect"));
            ui->networkConnect->setVisible(true);
            delete ap;
        }
    }
}

void InfoPaneDropdown::on_networkConnect_clicked()
{
    QDBusObjectPath device = ui->networkList->selectedItems().first()->data(Qt::UserRole + 1).value<QDBusObjectPath>();
    QDBusObjectPath accessPoint = ui->networkList->selectedItems().first()->data(Qt::UserRole).value<QDBusObjectPath>();

    if (ui->networkList->selectedItems().first()->data(Qt::UserRole + 2).toBool()) { //Already connected, disconnect from this network
        QDBusInterface *d = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus(), this);
        d->call("Disconnect");
        delete d;
    } else { //Not connected, connect to this network
        QDBusMessage message;
        if (ui->networkList->selectedItems().first()->data(Qt::UserRole + 3).toBool()) { //This network is already known
            message = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "ActivateConnection");

            QVariantList arguments;
            arguments.append(QVariant::fromValue(QDBusObjectPath("/")));
            arguments.append(QVariant::fromValue(device));
            arguments.append(QVariant::fromValue(accessPoint));
            message.setArguments(arguments);
        } else {
            QDBusInterface *ap = new QDBusInterface("org.freedesktop.NetworkManager", accessPoint.path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus(), this);
            uint wpaFlags = ap->property("WpaFlags").toUInt();

            QMap<QString, QVariantMap> connection;

            if (wpaFlags != 0) {
                QVariantMap wireless;
                wireless.insert("security", "802-11-wireless-security");
                connection.insert("802-11-wireless", wireless);

                QVariantMap wirelessSecurity;
                if (wpaFlags == 0x1 || wpaFlags == 0x2) { //WEP Authentication
                    wirelessSecurity.insert("key-mgmt", "none");
                    wirelessSecurity.insert("auth-alg", "shared");
                    wirelessSecurity.insert("wep-key0", ui->networkKey->text());
                } else { //WPA Authentication
                    wirelessSecurity.insert("key-mgmt", "wpa-psk");
                    wirelessSecurity.insert("psk", ui->networkKey->text());
                }
                connection.insert("802-11-wireless-security", wirelessSecurity);
            }
            message = QDBusMessage::createMethodCall("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "AddAndActivateConnection");
            QVariantList arguments;

            arguments.append(QVariant::fromValue(connection));
            arguments.append(QVariant::fromValue(device));
            arguments.append(QVariant::fromValue(accessPoint));

            message.setArguments(arguments);
            delete ap;
        }
        QDBusMessage reply = QDBusConnection::systemBus().call(message);
        qDebug() << reply.errorMessage();

        ui->networkKey->setText("");
    }
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
        ui->pushButton_2->setText("Start");
        ui->pushButton_3->setVisible(false);
        emit timerVisibleChanged(false);
        emit timerEnabledChanged(true);
    }
    ui->pushButton_2->setText("Pause");
    timeUntilTimeout = time;
    ui->label_7->setText(ui->timeEdit->text());
    ui->timeEdit->setVisible(false);
    ui->label_7->setVisible(true);
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [=]() {
        timeUntilTimeout = timeUntilTimeout.addSecs(-1);
        if (timeUntilTimeout == QTime(0, 0, 0)) {
            if (timerNotificationId != 0 ) {
                notificationEngine->CloseNotification(timerNotificationId);
            }
            timerNotificationId = notificationEngine->Notify("theShell", 0, "", "Timer Elapsed",
                                      "Your timer has completed.",
                                      QStringList(), QVariantMap(), 0);
            ui->timeEdit->setVisible(true);
            ui->label_7->setVisible(false);
            ui->pushButton_2->setText("Start");

            timer->stop();
            delete timer;
            timer = NULL;
            emit timerVisibleChanged(false);

            QMediaPlaylist* playlist = new QMediaPlaylist();

            if (ui->timerToneSelect->currentText() == "Happy Bee") {
                playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/happybee")));
            } else if (ui->timerToneSelect->currentText() == "Playing in the Dark") {
                playlist->addMedia(QMediaContent(QUrl("qrc:/sounds/tones/playinginthedark")));
            }
            playlist->setPlaybackMode(QMediaPlaylist::Loop);
            ringtone->setPlaylist(playlist);
            ringtone->play();
        } else {
            ui->label_7->setText(timeUntilTimeout.toString("HH:mm:ss"));
            emit timerChanged(timeUntilTimeout.toString("HH:mm:ss"));
        }
    });
    timer->start();
    emit timerChanged(timeUntilTimeout.toString("HH:mm:ss"));
}

void InfoPaneDropdown::notificationClosed(int id, int reason) {
    if (id == timerNotificationId) {
        ringtone->stop();
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
            ui->pushButton_2->setText("Resume");
            emit timerEnabledChanged(false);
        } else {
            timer->start();
            ui->pushButton_3->setVisible(false);
            ui->label_7->setEnabled(true);
            ui->pushButton_2->setText("Pause");
            emit timerEnabledChanged(true);
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
    ui->pushButton_2->setText("Start");
    ui->pushButton_3->setVisible(false);
    emit timerVisibleChanged(false);
    emit timerEnabledChanged(true);
}

void InfoPaneDropdown::on_pushButton_7_clicked()
{
    changeDropDown(Settings);
}

void InfoPaneDropdown::on_pushButton_8_clicked()
{
    QProcess::startDetached("systemsettings5");
    this->close();
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
    settings.setValue("startup/autostart", ui->lineEdit_2->text());
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
        iconLabel->setPixmap(icon.pixmap(22, 22));
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

        ui->notificationsList->layout()->addWidget(frame);
        ui->noNotifications->setVisible(false);
        ui->clearAllNotifications->setVisible(true);
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
        ui->noNotifications->setVisible(true);
        ui->clearAllNotifications->setVisible(false);
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

void InfoPaneDropdown::batteryLevelChanged(int battery) {
    ui->currentBattery->setText("Current Battery Percentage: " + QString::number(battery));
}

void InfoPaneDropdown::on_printLabel_clicked()
{
    changeDropDown(Print);
}

bool InfoPaneDropdown::isQuietOn() {
    return ui->QuietCheck->isChecked();
}

bool InfoPaneDropdown::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = (QKeyEvent*) event;
        if (keyEvent->key() == Qt::Key_Left) {
            if (ui->pushButton_5->isEnabled()) {
                on_pushButton_5_clicked();
            }
            return true;
        } else if (keyEvent->key() == Qt::Key_Right) {
            if (ui->pushButton_6->isEnabled()) {
                on_pushButton_6_clicked();
            }
            return true;
        }
    }
    return false;
}

void InfoPaneDropdown::on_resetButton_clicked()
{
    if (QMessageBox::warning(this, "Reset theShell",
                             "All settings will be reset to default, and you will be logged out. "
                             "Are you sure you want to do this?", QMessageBox::Yes | QMessageBox::No,
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
    if (dragRect.bottom() >= screenGeometry.bottom()) {
        dragRect.moveTo(screenGeometry.left(), screenGeometry.top());
    }
    this->setGeometry(dragRect);

    mouseClickPoint = event->globalY();
    event->accept();
}

void InfoPaneDropdown::mouseReleaseEvent(QMouseEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (initialPoint - 5 > mouseClickPoint && initialPoint + 5 < mouseClickPoint) {
        QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
        a->setStartValue(this->geometry());
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
        a->setEasingCurve(QEasingCurve::OutCubic);
        a->setDuration(500);
        connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
        a->start();
    } else {
        if (mouseMovedUp) {
            this->close();
        } else {
            QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
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
