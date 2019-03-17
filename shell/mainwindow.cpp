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

#include "mainwindow.h"
#include "ui_mainwindow.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);
extern DbusEvents* DBusEvents;
extern TutorialWindow* TutorialWin;
extern AudioManager* AudioMan;
extern NativeEventFilter* NativeFilter;
extern float getDPIScaling();
extern LocationServices* locationServices;
extern UPowerDBus* updbus;
extern ScreenRecorder* screenRecorder;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->StatusBarFrame->setAttribute(Qt::WA_AcceptTouchEvents, true);

    ui->openMenu->setIconSize(QSize(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->openMenuCompact->setIconSize(QSize(32 * getDPIScaling(), 32 * getDPIScaling()));
    QSize ic16(16 * getDPIScaling(), 16 * getDPIScaling());
    ui->brightnessButton->setIconSize(ic16);
    ui->volumeButton->setIconSize(ic16);

    //Prepare status bar
    ui->StatusBarFrame->setParent(this);
    ui->StatusBarFrame->setFixedWidth(this->width());
    ui->StatusBarFrame->setFixedHeight(24 * getDPIScaling());

    //Set the menu of the MPRIS Media Player selection to a new menu.
    //Items will be populated during the update event.
    QMenu* mprisSelectionMenu = new QMenu();
    ui->mprisSelection->setMenu(mprisSelectionMenu);
    connect(mprisSelectionMenu, &QMenu::aboutToShow, [=]() {
        pauseMprisMenuUpdate = true;
        lockMovement("MPRIS");
    });
    connect(mprisSelectionMenu, &QMenu::aboutToHide, [=]() {
        pauseMprisMenuUpdate = false;
        unlockMovement("MPRIS");
    });

    //Connect signals related to multiple monitor management
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));

    remakeBar();

    //Set up bar movement
    /*barAnim = new tPropertyAnimation(this, "geometry");
    barAnim->setDuration(500);
    barAnim->setEasingCurve(QEasingCurve::OutCubic);*/

    //Create the gateway and set required flags
    gatewayMenu = new Menu(this);
    gatewayMenu->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(gatewayMenu, &Menu::menuClosing, [=]() {
        unlockMovement("Gateway closing");
    });

    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    reloadBar();

    taskbarManager = new TaskbarManager;
    connect(taskbarManager, SIGNAL(updateWindow(WmWindow)), this, SLOT(updateWindow(WmWindow)));
    connect(taskbarManager, SIGNAL(deleteWindow(WmWindow)), this, SLOT(deleteWindow(WmWindow)));
    taskbarManager->ReloadWindows();

    //Create the update event timer and start it
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(doUpdate()));
    connect(timer, SIGNAL(timeout()), taskbarManager, SLOT(ReloadWindows()));
    timer->start();

    infoPane = new InfoPaneDropdown(this->winId());
    connect(infoPane, SIGNAL(timerEnabledChanged(bool)), this, SLOT(setTimerEnabled(bool)));
    connect(infoPane, &InfoPaneDropdown::batteryStretchChanged, [=](bool isOn) {
        if (isOn) {
            timer->setInterval(1000);
        } else {
            timer->setInterval(100);
        }
    });
    connect(infoPane, SIGNAL(updateStrutsSignal()), this, SLOT(updateStruts()));
    connect(infoPane, SIGNAL(updateBarSignal()), this, SLOT(reloadBar()));
    connect(infoPane, &InfoPaneDropdown::flightModeChanged, [=](bool flight) {
        ui->StatusBarFlight->setVisible(flight);
    });
    connect(infoPane, &InfoPaneDropdown::redshiftEnabledChanged, [=](bool enabled) {
        ui->StatusBarRedshift->setVisible(enabled);
    });
    connect(infoPane, &InfoPaneDropdown::keyboardLayoutChanged, [=](QString code) {
        ui->keyboardButton->setText(code);
    });
    connect(infoPane, &InfoPaneDropdown::newKeyboardLayoutMenuAvailable, [=](QMenu* menu) {
        if (menu == nullptr) {
            ui->keyboardButton->setVisible(false);
        } else {
            ui->keyboardButton->setVisible(true);
            ui->keyboardButton->setMenu(menu);

            connect(menu, &QMenu::aboutToShow, [=] {
                lockMovement("Keyboard Layout");
            });
            connect(menu, &QMenu::aboutToHide, [=] {
                unlockMovement("Keyboard Layout");
            });
        }
    });
    connect(infoPane, &InfoPaneDropdown::statusBarProgress, [=](QString title, QString description, int progress) {
        this->statusBarPercentage = progress;
        if (ui->StatusBarProgressTitle->text() != title || ui->StatusBarProgressDescription->text() != description || !statusBarProgressTimer->isActive()) {
            ui->StatusBarProgressTitle->setText(title);
            ui->StatusBarProgressDescription->setText(description);

            //Animate in the progress description
            showStatusBarProgress(true);
            if (statusBarProgressTimer->isActive()) statusBarProgressTimer->stop();
            statusBarProgressTimer->start();
        }

        if (progress == 0) {
            ui->StatusBarProgressSpinner->setVisible(true);
        } else {
            ui->StatusBarProgressSpinner->setVisible(false);
        }

        ui->StatusBarFrame->update();
        ui->StatusBarProgressFrame->update();
    });
    connect(infoPane, &InfoPaneDropdown::statusBarProgressFinished, [=](QString title, QString description) {
        this->statusBarPercentage = -2;
        //ui->StatusBarProgressTitle->setText(title);
        //ui->StatusBarProgressDescription->setText(description);

        //Animate in the progress description
        //showStatusBarProgress(true);
        showStatusBarProgress(false);
        if (statusBarProgressTimer->isActive()) statusBarProgressTimer->stop();

        ui->StatusBarFrame->update();
        ui->StatusBarProgressFrame->update();
    });
    connect(infoPane, &InfoPaneDropdown::newChunk, [=](QWidget* chunk) {
        ui->chunksLayout->addWidget(chunk);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        ui->chunksLayout->addWidget(line);

        ChunkWatcher* watcher = new ChunkWatcher(chunk);
        connect(watcher, &ChunkWatcher::visibilityChanged, [=](bool isVisible) {
            line->setVisible(isVisible);
        });
        line->setVisible(chunk->isVisible());
    });
    connect(infoPane, &InfoPaneDropdown::newSnack, [=](QWidget* snack) {
        ui->StatusBarSnacks->addWidget(snack);
    });
    ui->StatusBarRedshift->setVisible(false);
    ui->keyboardButton->setVisible(false);

    connect(ui->openMenuCompact, SIGNAL(customContextMenuRequested(QPoint)), ui->openMenu, SIGNAL(customContextMenuRequested(QPoint)));

    connect(updbus, &UPowerDBus::updateDisplay, [=](QString display) {
        if (updbus->hasBattery()) {
            if (updbus->hasPCBattery()) {
                ui->batteryIcon->setVisible(true);
            } else {
                ui->batteryIcon->setVisible(false);
            }
            ui->batteryFrame->setVisible(true);
            ui->batteryLabel->setText(display);

            QString iconName;
            if (updbus->charging()) {
                if (updbus->currentBattery() < 10) {
                    iconName = "battery-charging-empty";
                } else if (updbus->currentBattery() < 30) {
                    iconName = "battery-charging-020";
                } else if (updbus->currentBattery() < 50) {
                    iconName = "battery-charging-040";
                } else if (updbus->currentBattery() < 70) {
                    iconName = "battery-charging-060";
                } else if (updbus->currentBattery() < 90) {
                    iconName = "battery-charging-080";
                } else {
                    iconName = "battery-charging-100";
                }
            } else if (updbus->powerStretch()) {
                if (updbus->currentBattery() < 10) {
                    iconName = "battery-stretch-empty";
                } else if (updbus->currentBattery() < 30) {
                    iconName = "battery-stretch-020";
                } else if (updbus->currentBattery() < 50) {
                    iconName = "battery-stretch-040";
                } else if (updbus->currentBattery() < 70) {
                    iconName = "battery-stretch-060";
                } else if (updbus->currentBattery() < 90) {
                    iconName = "battery-stretch-080";
                } else {
                    iconName = "battery-stretch-100";
                }
            } else {
                if (updbus->currentBattery() < 10) {
                    iconName = "battery-empty";
                } else if (updbus->currentBattery() < 30) {
                    iconName = "battery-020";
                } else if (updbus->currentBattery() < 50) {
                    iconName = "battery-040";
                } else if (updbus->currentBattery() < 70) {
                    iconName = "battery-060";
                } else if (updbus->currentBattery() < 90) {
                    iconName = "battery-080";
                } else {
                    iconName = "battery-100";
                }
            }

            ui->StatusBarBattery->setVisible(true);
            ui->StatusBarBattery->setPixmap(QIcon::fromTheme(iconName).pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
            ui->batteryIcon->setPixmap(QIcon::fromTheme(iconName).pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
        } else {
            ui->batteryFrame->setVisible(false);
            ui->StatusBarBattery->setVisible(false);
        }
    });
    QTimer::singleShot(0, [=] {
        updbus->DeviceChanged(updbus->allDevices);
    });

    seperatorWidget = new QWidget();
    seperatorWidget->setParent(this);
    seperatorWidget->raise();
    seperatorWidget->installEventFilter(this);

    DBusEvents = new DbusEvents();

    QString loginSoundPath = settings.value("sounds/login", "").toString();
    if (loginSoundPath == "") {
        loginSoundPath = "/usr/share/sounds/contemporary/login.ogg";
        settings.setValue("sounds/login", loginSoundPath);
    }

    playSound(QUrl::fromLocalFile(loginSoundPath));

    ui->timer->setVisible(false);
    ui->timerIcon->setVisible(false);
    ui->screenRecordingFrame->setVisible(false);
    ui->timerIcon->setPixmap(QIcon::fromTheme("chronometer").pixmap(ic16));
    ui->StatusBarMpris->setVisible(false);
    ui->StatusBarMprisIcon->setVisible(false);
    ui->StatusBarQuietMode->setVisible(false);
    ui->StatusBarLocation->setVisible(false);
    ui->LocationIndication->setVisible(false);
    ui->StatusBarRecording->setVisible(false);
    ui->StatusBarLocation->setPixmap(QIcon::fromTheme("gps").pixmap(ic16));
    ui->LocationIndication->setPixmap(QIcon::fromTheme("gps").pixmap(ic16));
    ui->StatusBarFlight->setPixmap(QIcon::fromTheme("flight-mode").pixmap(ic16));
    ui->StatusBarFlight->setVisible(settings.value("flightmode/on", false).toBool());
    ui->StatusBarFrame->installEventFilter(this);
    ui->StatusBarRedshift->setPixmap(QIcon::fromTheme("redshift-on").pixmap(ic16));
    ui->StatusBarRecording->setPixmap(QIcon::fromTheme("media-record").pixmap(ic16));

    ((QBoxLayout*) ui->centralWidget->layout())->removeWidget(ui->StatusBarHoverFrame);
    ui->StatusBarHoverFrame->setParent(this);
    ui->StatusBarHoverFrame->resize(ui->StatusBarFrame->size());
    ui->StatusBarHoverFrame->move(0, -ui->StatusBarHoverFrame->height());
    ui->StatusBarHoverFrame->installEventFilter(this);
    ui->StatusBarFrame->setMouseTracking(true);

    ((QBoxLayout*) ui->centralWidget->layout())->removeWidget(ui->StatusBarProgressFrame);
    ui->StatusBarProgressFrame->setParent(this);
    ui->StatusBarProgressFrame->resize(ui->StatusBarFrame->size());
    ui->StatusBarProgressFrame->move(0, -ui->StatusBarHoverFrame->height());
    ui->StatusBarProgressFrame->installEventFilter(this);
    ui->StatusBarProgressFrame->setMouseTracking(true);

    QPalette hoverFramePal = this->palette();
    hoverFramePal.setColor(QPalette::Window, hoverFramePal.color(QPalette::Highlight));
    hoverFramePal.setColor(QPalette::WindowText, hoverFramePal.color(QPalette::WindowText));
    ui->StatusBarHoverFrame->setPalette(hoverFramePal);

    statusBarOpacityEffect = new QGraphicsOpacityEffect();
    statusBarOpacityEffect->setOpacity(0);
    ui->StatusBarFrame->setGraphicsEffect(statusBarOpacityEffect);

    connect(locationServices, &LocationServices::locationUsingChanged, [=](bool location) {
        ui->StatusBarLocation->setVisible(location);
        ui->LocationIndication->setVisible(location);
    });

    ui->volumeSlider->setVisible(false);
    ui->brightnessSlider->setVisible(false);
    ui->mprisFrame->setVisible(false);

    this->setFocusPolicy(Qt::NoFocus);

    QMenu* quietModeMenu = new QMenu();
    quietModeMenu->addSection(tr("Quiet Mode"));
    quietModeMenu->addAction(ui->actionNone);
    quietModeMenu->addAction(ui->actionCriticalOnly);
    quietModeMenu->addAction(ui->actionNotifications);
    quietModeMenu->addAction(ui->actionMute);
    connect(quietModeMenu, &QMenu::aboutToShow, [=] {
        lockMovement("Quiet Mode");
    });
    connect(quietModeMenu, &QMenu::aboutToHide, [=] {
        unlockMovement("Quiet Mode");
    });
    ui->volumeButton->setMenu(quietModeMenu);

    connect(AudioMan, &AudioManager::QuietModeChanged, [=](AudioManager::quietMode mode) {
        switch (mode) {
            case AudioManager::none:
                ui->volumeButton->setIcon(QIcon::fromTheme("audio-volume-high"));
                ui->StatusBarQuietMode->setVisible(false);
                break;
            case AudioManager::critical:
                ui->volumeButton->setIcon(QIcon::fromTheme("quiet-mode-critical-only"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("quiet-mode-critical-only").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
                ui->StatusBarQuietMode->setVisible(true);
                break;
            case AudioManager::notifications:
                ui->volumeButton->setIcon(QIcon::fromTheme("quiet-mode"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("quiet-mode").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
                ui->StatusBarQuietMode->setVisible(true);
                break;
            case AudioManager::mute:
                ui->volumeButton->setIcon(QIcon::fromTheme("audio-volume-muted"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("audio-volume-muted").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
                ui->StatusBarQuietMode->setVisible(true);
                break;
        }
        if (mode == AudioManager::none) {
        } else if (mode == AudioManager::notifications) {
        } else if (mode == AudioManager::critical) {

        } else {
        }
    });
    connect(screenRecorder, &ScreenRecorder::stateChanged, [=](ScreenRecorder::State state) {
        if (state == ScreenRecorder::Recording) {
            ui->StatusBarRecording->setVisible(true);
            ui->screenRecordingFrame->setVisible(true);
            ui->screenRecordingProcessingSpinner->setVisible(false);
            ui->stopRecordingButton->setVisible(true);
            ui->screenRecordingActiveLabel->setText(tr("Recording Screen"));
        } else if (state == ScreenRecorder::Processing) {
            ui->StatusBarRecording->setVisible(false);
            ui->screenRecordingFrame->setVisible(true);
            ui->screenRecordingProcessingSpinner->setVisible(true);
            ui->stopRecordingButton->setVisible(false);
            ui->screenRecordingActiveLabel->setText(tr("Processing Screen Recording..."));

            ui->screenRecordingProcessingSpinner->setFixedWidth(16 * getDPIScaling());
        } else {
            ui->StatusBarRecording->setVisible(false);
            ui->screenRecordingFrame->setVisible(false);
        }
    });

    ui->StatusBarFrame->setVisible(false);
    ui->StatusBarHoverFrame->setVisible(false);
    ui->StatusBarProgressFrame->setVisible(false);

    statusBarProgressTimer = new QTimer();
    statusBarProgressTimer->setInterval(5000);
    connect(statusBarProgressTimer, &QTimer::timeout, [=] {
        this->showStatusBarProgress(!ui->StatusBarProgressFrame->isVisible());
        if (statusBarPercentage == -2) statusBarProgressTimer->stop();
    });
    ui->StatusBarProgressSpinner->setFixedSize(QSize(16, 16) * getDPIScaling());

    //ui->infoScrollArea->setFixedHeight(ui->InfoScrollWidget->height());

    #ifdef BLUEPRINT
        //Apply Blueprint branding
        ui->openMenu->setIcon(QIcon(":/icons/icon-bp.svg"));
        ui->openMenuCompact->setIcon(QIcon(":/icons/icon-bp.svg"));
    #endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::remakeBar() {
    //This is a hack...
    qDebug() << "barAnim was destroyed :(";
    barAnim = new tVariantAnimation();
    barAnim->setDuration(500);
    barAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(barAnim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setGeometry(value.toRect());
    });
    connect(barAnim, &tVariantAnimation::destroyed, [=] {
        remakeBar();
    });
}

void MainWindow::updateWindow(WmWindow window) {

    FadeButton *button;
    if (buttonWindowMap.keys().contains(window.WID())) {
        button = buttonWindowMap.value(window.WID());
    } else {
        //Create a new button
        button = new FadeButton();

        //Add the button to the layout
        ui->windowList->layout()->addWidget(button);
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(button, &QPushButton::customContextMenuRequested, [=](const QPoint &pos) {
            QMenu* menu = new QMenu();

            menu->addSection(window.icon(), tr("For %1").arg(window.title()));
            menu->addAction(QIcon::fromTheme("window-close"), tr("Close"), [=] {
                sendMessageToRootWindow("_NET_CLOSE_WINDOW", window.WID());
            });

            /*//Determine if process is theShell
            if (window.PID() != QApplication::applicationPid()) {
                //Determine if process is suspended
                QFile wchan("/proc/" + QString::number(window.PID()) + "/wchan");
                if (wchan.exists()) {
                    wchan.open(QFile::ReadOnly);
                    if (wchan.readAll() == "do_signal_stop") { //Stopped
                        menu->addAction("Resume", [=] {
                            kill(window.PID(), SIGCONT);
                        });
                    } else { //Running
                        menu->addAction("Stop", [=] {
                            kill(window.PID(), SIGTSTP);

                            //Determine if process is suspended
                            QFile wchan("/proc/" + QString::number(window.PID()) + "/wchan");
                            if (wchan.exists()) {
                                wchan.open(QFile::ReadOnly);
                                if (wchan.readAll() != "do_signal_stop") { //Not stopped
                                    kill(window.PID(), SIGSTOP);
                                }
                                wchan.close();
                            }
                        });
                    }
                    wchan.close();
                }
            }*/

            lockMovement("Window Menu");
            menu->exec(button->mapToGlobal(pos));
            unlockMovement("Window Menu");
        });
        connect(button, SIGNAL(clicked(bool)), this, SLOT(ActivateWindow()));
    }

    button->setProperty("windowid", QVariant::fromValue(window.WID()));
    button->setProperty("desktop", QVariant::fromValue(window.desktop()));
    if (settings.value("bar/showText", true).toBool()) {
        button->setFullText(window.title().replace("&", "&&"));
    } else {
        button->setFullText("");
    }

    if (window.isMinimized() /*|| (currentDesktop != window.desktop() && window.desktop() != 0xFFFFFFFF)*/) {
        button->setFade(true);
    } else {
        button->setFade(false);
    }
    /*if (active == window.WID()) {
        button->setCheckable(true);
        button->setChecked(true);
        connect(button, &FadeButton::toggled, [=]() {
            button->setChecked(true);
        });
    }*/
    button->setIcon(window.icon());

    //If window is requesting attention, highlight it
    if (window.attention()) {
        button->setStyleSheet("background-color: #AAAA0000;");
    } else {
        button->setStyleSheet("");
    }

    if (!buttonWindowMap.keys().contains(window.WID())) {
        button->animateIn();
    }
    buttonWindowMap.insert(window.WID(), button);
}

void MainWindow::deleteWindow(WmWindow window) {
    if (buttonWindowMap.contains(window.WID())) {
        FadeButton* button = buttonWindowMap.value(window.WID());
        button->animateOut();
        buttonWindowMap.remove(window.WID());
        QTimer::singleShot(2000, [=] {
            button->deleteLater();
        });
    }
}

void MainWindow::DBusNewService(QString name) {
    if (name.startsWith("org.mpris.MediaPlayer2.")) {
        if (!mprisDetectedApps.contains(name)) {
            if (mprisCurrentAppName == "") {
                setMprisCurrentApp(name);
            }
        }
    }
}

void MainWindow::updateMpris(QString interfaceName, QMap<QString, QVariant> properties, QStringList changedProperties) {
    if (interfaceName == "org.mpris.MediaPlayer2.Player") {
        if (properties.keys().contains("Metadata")) {
            QVariantMap replyData;
            properties.value("Metadata").value<QDBusArgument>() >> replyData;
            QString title = "";
            QString artist = "";

            if (replyData.contains("xesam:title")) {
                title = replyData.value("xesam:title").toString();
            }
            this->mprisTitle = title;

            if (replyData.contains("xesam:artist")) {
                QStringList artists = replyData.value("xesam:artist").toStringList();
                for (QString art : artists) {
                    artist.append(art + ", ");
                }
                artist.remove(artist.length() - 2, 2);
            }
            this->mprisArtist = artist;

            if (replyData.contains("xesam:album")) {
                this->mprisAlbum = replyData.value("xesam:album").toString();
            } else {
                this->mprisAlbum = "";
            }

            QString songName;
            if (title == "") {
                QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
                IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

                QEventLoop loop;
                QDBusPendingCallWatcher watcher(QDBusConnection::sessionBus().asyncCall(IdentityRequest, 100));
                connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &loop, SLOT(quit()));
                loop.exec();

                //QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
                songName = watcher.reply().arguments().first().value<QDBusVariant>().variant().toString();
            } else {
                if (artist == "") {
                    songName = title;
                } else {
                    songName = artist + " · " + title;
                }
            }

            ui->mprisSongName->setText(songName);
            ui->StatusBarMpris->setText(songName);
            ui->StatusBarMpris->setVisible(true);
            ui->StatusBarMprisIcon->setVisible(true);
            ui->StatusBarFrame->setFixedWidth(this->width());
        }

        if (properties.keys().contains("PlaybackStatus")) {
            if (properties.value("PlaybackStatus").toString() == "Playing") {
                ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
                ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
                mprisPlaying = true;
            } else {
                ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));
                ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-pause").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
                mprisPlaying = false;
            }
        }
    }
}

void MainWindow::pullDownGesture() {
    if (lockHide) {
        on_date_clicked();
    } else {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        barAnim->setStartValue(this->geometry());
        barAnim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width() + 1, this->height()));
        barAnim->start();

        lockMovement("Pull Down");
        QTimer* timer = new QTimer();
        timer->setSingleShot(true);
        timer->setInterval(3000);
        connect(timer, &QTimer::timeout, [=]() {
            unlockMovement("Pull Down");
            timer->deleteLater();
        });
        timer->start();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}

void MainWindow::on_openMenu_clicked()
{
    openMenu();
}

void MainWindow::doUpdate() {
    /*{
        QFile f("/proc/self/status");
        f.open(QFile::ReadOnly);

        QByteArray a = f.readAll();
        QBuffer buf(&a);
        buf.open(QBuffer::ReadOnly);
        while (!buf.atEnd()) {
            QString l = buf.readLine();
            if (l.startsWith("VmRSS:")) {
                QString mem = l.split(" ", QString::SkipEmptyParts).at(1);
                int m = mem.toInt();
                if (m > 500000) {
                    qDebug() << "Memory!!!!!";
                }
            }
        }
    }*/

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    Display* d = QX11Info::display();

    //Get the current desktop
    int currentDesktop = 0;
    {
        unsigned long *desktop;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_CURRENT_DESKTOP", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktop);
        if (retval == 0 && desktop != 0) {
            currentDesktop = *desktop;
        }
        XFree(desktop);
    }
    ui->desktopName->setProperty("desktopIndex", currentDesktop);

    //Get the current desktop
    {
        unsigned char *desktopNames;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_DESKTOP_NAMES", False), 0, 1024, False,
                                        XInternAtom(d, "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, (unsigned char**) &desktopNames);
        if (retval == 0 && desktopNames != 0) {
            QByteArray characterBytes = QByteArray::fromRawData((char *) desktopNames, items);
            QList<QByteArray> nameList = characterBytes.split(0x0);
            if (nameList.count() <= currentDesktop) {
                ui->desktopName->setText(tr("Desktop %1").arg(QString::number(currentDesktop + 1)));
            } else {
                ui->desktopName->setText(ui->desktopName->fontMetrics().elidedText(QString(nameList.at(currentDesktop)), Qt::ElideRight, 200));
            }
        }
        XFree(desktopNames);
    }

    //Get the number of desktops
    int numOfDesktops = 0;
    {
        unsigned long *desktops;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktops);
        if (retval == 0 && desktops != 0) {
            numOfDesktops = *desktops;
        }
        XFree(desktops);
    }
    if (numOfDesktops == 1) {
        ui->desktopsFrame->setVisible(false);
    } else {
        ui->desktopsFrame->setVisible(true);
    }

    /*
    QList<WmWindow> wlist;

    int hideTop = screenGeometry.y();
    int okCount = 0;

    Display* d = QX11Info::display();

    int currentDesktop = 0;
    { //Get the current desktop
        unsigned long *desktop;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_CURRENT_DESKTOP", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktop);
        if (retval == 0 && desktop != 0) {
            currentDesktop = *desktop;
        }
        XFree(desktop);
    }

    ui->desktopName->setProperty("desktopIndex", currentDesktop);

    { //Get the desktop names
        unsigned char *desktopNames;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_DESKTOP_NAMES", False), 0, 1024, False,
                                        XInternAtom(d, "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, (unsigned char**) &desktopNames);
        if (retval == 0 && desktopNames != 0) {
            QByteArray characterBytes = QByteArray::fromRawData((char *) desktopNames, items);
            QList<QByteArray> nameList = characterBytes.split(0x0);
            if (nameList.count() <= currentDesktop) {
                ui->desktopName->setText(tr("Desktop %1").arg(QString::number(currentDesktop + 1)));
            } else {
                ui->desktopName->setText(ui->desktopName->fontMetrics().elidedText(QString(nameList.at(currentDesktop)), Qt::ElideRight, 200));
            }
        }
        XFree(desktopNames);
    }

    Window active;
    { //Get the active window
        Window *activeWin;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_ACTIVE_WINDOW", False), 0, 1024, False,
                                        AnyPropertyType, &ReturnType, &format, &items, &bytes, (unsigned char**) &activeWin);
        if (retval == 0 && activeWin != 0) {
             active = *activeWin;
        }
        XFree(activeWin);
    }

    int numOfDesktops = 0;
    { //Get the number of desktops
        unsigned long *desktops;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktops);
        if (retval == 0 && desktops != 0) {
            numOfDesktops = *desktops;
        }
        XFree(desktops);
    }
    if (numOfDesktops == 1) {
        ui->desktopsFrame->setVisible(false);
    } else {
        ui->desktopsFrame->setVisible(true);
    }

    QList<Window> TopWindows;

    Atom WindowListType;
    int format;
    unsigned long items, bytes;
    unsigned char *data;
    XGetWindowProperty(d, DefaultRootWindow(d), XInternAtom(d, "_NET_CLIENT_LIST", true), 0L, (~0L),
                                    False, AnyPropertyType, &WindowListType, &format, &items, &bytes, &data);

    quint64 *windows = (quint64*) data;
    for (unsigned int i = 0; i < items; i++) {
        TopWindows.append((Window) windows[i]);

    }
    XFree(data);

    int demandAttention = 0; //Space for storing number of windows that demand attention
    for (Window win : TopWindows) {
        XWindowAttributes attributes;

        int retval = XGetWindowAttributes(d, win, &attributes);
        unsigned long items, bytes;
        unsigned char *netWmName;
        int format;
        Atom ReturnType;
        retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_VISIBLE_NAME", False), 0, 1024, False,
                           XInternAtom(d, "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, &netWmName);
        if (retval != 0 || netWmName == 0x0) {
            retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_NAME", False), 0, 1024, False,
                               AnyPropertyType, &ReturnType, &format, &items, &bytes, &netWmName);
            if (retval != 0) {
                retval = 1;
            }
        }
        if (retval == 0) {
            WmWindow w;

            int windowx, windowy;
            Window child;
            retval = XTranslateCoordinates(d, win, RootWindow(d, 0), 0, 0, &windowx, &windowy, &child);

            QString title;
            if (netWmName) {
                title = QString::fromUtf8((char *) netWmName);
                XFree(netWmName);
            }

            unsigned long *pidPointer;
            unsigned long pitems, pbytes;
            int pformat;
            Atom pReturnType;
            int retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_PID", False), 0, 1024, False,
                                            XA_CARDINAL, &pReturnType, &pformat, &pitems, &pbytes, (unsigned char**) &pidPointer);
            if (retval == 0) {
                if (pidPointer != 0) {
                    unsigned long pid = *pidPointer;
                    w.setPID(pid);
                }
            }
            XFree(pidPointer);

            {
                unsigned long *desktop;
                unsigned long items, bytes;
                int format;
                Atom ReturnType;
                int retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_DESKTOP", False), 0, 1024, False,
                                                XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktop);
                if (retval == 0 && desktop != 0) {
                    w.setDesktop(*desktop);
                }
                XFree(desktop);
            }


            {
                bool noIcon = false;
                unsigned long icItems, icBytes;
                unsigned char *icon;
                int icFormat;
                Atom icReturnType;

                unsigned char *ret;
                int width, height;

                //Get all icon sizes
                //QMap<int, QSize> iconSizes;

                //do {
                    retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_ICON", False), 0, 1, False,
                                       XA_CARDINAL, &icReturnType, &icFormat, &icItems, &icBytes, &ret);
                    if (ret == 0x0) {
                        noIcon = true;
                    } else {
                        width = *(int*) ret;
                        XFree(ret);
                    }

                    retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_ICON", False), 1, 1, False,
                                       XA_CARDINAL, &icReturnType, &icFormat, &icItems, &icBytes, &ret);

                    if (ret == 0x0) {
                        noIcon = true;
                    } else {
                        height = *(int*) ret;
                        XFree(ret);
                    }

                    //iconSizes.insert(offset, QSize(width, height));
                    //offset += width * height * 4 + 2;
                //} while (!noIcon || icBytes == width * height * 4);

                if (!noIcon) {
                    /*QSize currentSize = QSize(0, 0);

                    for (int offsets : iconSizes.keys()) {
                        if (currentSize == QSize(0, 0) || (currentSize.width() > iconSizes.value(offsets).width() && currentSize.height() > iconSizes.value(offsets).height())) {
                            currentSize = iconSizes.value(offsets);
                            imageOffset = offsets;
                        }
                    }*

                    //width = currentSize.width();
                    //height = currentSize.height();

                    retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_ICON", False), 2, width * height * 4, False,
                                       XA_CARDINAL, &icReturnType, &icFormat, &icItems, &icBytes, &icon);

                    QImage image(16, 16, QImage::Format_ARGB32);

                    float widthSpacing = (float) width / (float) 16;
                    float heightSpacing = (float) height / (float) 16;

                    for (int y = 0; y < 16; y++) {
                        for (int x = 0; x < 16 * 8; x = x + 8) {
                            unsigned long a, r, g, b;

                            b = (icon[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 0)]);
                            g = (icon[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 1)]);
                            r = (icon[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 2)]);
                            a = (icon[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 3)]);

                            QColor col = QColor(r, g, b, a);

                            image.setPixelColor(x / 8, y, col);
                        }
                    }

                    QPixmap iconPixmap(QPixmap::fromImage(image).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    w.setIcon(QIcon(iconPixmap));

                    XFree(icon);
                }
            }

            w.setTitle(title);
            w.setWID(win);

            bool addToList = true;
            if (w.PID() == QApplication::applicationPid() && w.title() != "Choose Background") {
                addToList = false;
                if (w.title() == "Ending Session") {
                    hideTop = 0;
                    lockHide = false;
                }
            }

            if (addToList) {
                bool skipTaskbar = false;

                {
                    Atom returnType;
                    int format;
                    unsigned long items, bytes;
                    Atom* atoms;

                    XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_STATE", False), 0, 1024, False,
                                       XA_ATOM, &returnType, &format, &items, &bytes, (unsigned char**) &atoms);

                    for (unsigned int i = 0; i < items; i++) {
                        if (atoms[i] == XInternAtom(d, "_NET_WM_STATE_HIDDEN", False)) {
                            w.setMinimized(true);
                        } else if (atoms[i] == XInternAtom(d, "_NET_WM_STATE_SKIP_TASKBAR", False)) {
                            skipTaskbar = true;
                        } else if (atoms[i] == XInternAtom(d, "_NET_WM_STATE_DEMANDS_ATTENTION", False)) {
                            w.setAttention(true);
                            demandAttention++;
                        }
                    }

                    XFree(atoms);
                }

                if (!skipTaskbar) {
                    if (settings.value("bar/showWindowsFromOtherDesktops", true).toBool() ||
                                     w.desktop() == currentDesktop) {
                        if (!w.isMinimized() && windowx >= this->x() &&
                                windowy - 50 <= screenGeometry.y() + this->height() &&
                                windowy - 50 - this->height() < hideTop && w.desktop() == currentDesktop) {
                            hideTop = windowy - 50 - this->height();
                        }

                        wlist.append(w);

                        for (WmWindow wi : windowList) {
                            if (wi.title() == w.title()) {
                                okCount++;
                                break;
                            }
                        }
                    }
                }
            }

        }
    }

    if (hideTop + this->height() <= screenGeometry.y()) {
        hideTop = screenGeometry.y() - this->height();
    }

    if (okCount != wlist.count() || wlist.count() < windowList.count() || demandAttention != attentionDemandingWindows ||
            oldDesktop != currentDesktop || oldActiveWindow != active) {

        windowList = wlist;

        QLayoutItem* item;
        while ((item = ui->windowList->layout()->takeAt(0)) != NULL) {
            ui->windowList->layout()->removeItem(item);
            delete item->widget();
            delete item;
        }
        for (WmWindow w : windowList) {
            FadeButton *button = new FadeButton();

            button->setProperty("windowid", QVariant::fromValue(w.WID()));
            button->setProperty("desktop", QVariant::fromValue(w.desktop()));
            if (settings.value("bar/showText", true).toBool()) {
                button->setFullText(w.title().replace("&", "&&"));
            }
            button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(button, &QPushButton::customContextMenuRequested, [=](const QPoint &pos) {
                QMenu* menu = new QMenu();

                menu->addSection(w.icon(), tr("For %1").arg(w.title()));
                menu->addAction(QIcon::fromTheme("window-close"), tr("Close"), [=] {
                    sendMessageToRootWindow("_NET_CLOSE_WINDOW", w.WID());
                });

                //Determine if process is theShell
                if (w.PID() != QApplication::applicationPid()) {
                    //Determine if process is suspended
                    QFile wchan("/proc/" + QString::number(w.PID()) + "/wchan");
                    if (wchan.exists()) {
                        wchan.open(QFile::ReadOnly);
                        if (wchan.readAll() == "do_signal_stop") { //Stopped
                            menu->addAction("Resume", [=] {
                                kill(w.PID(), SIGCONT);
                            });
                        } else { //Running
                            menu->addAction("Stop", [=] {
                                kill(w.PID(), SIGTSTP);

                                //Determine if process is suspended
                                QFile wchan("/proc/" + QString::number(w.PID()) + "/wchan");
                                if (wchan.exists()) {
                                    wchan.open(QFile::ReadOnly);
                                    if (wchan.readAll() != "do_signal_stop") { //Not stopped
                                        kill(w.PID(), SIGSTOP);
                                    }
                                    wchan.close();
                                }
                            });
                        }
                        wchan.close();
                    }
                }

                lockHide = true;
                menu->exec(button->mapToGlobal(pos));
                lockHide = false;
            });
            if (w.isMinimized() || (currentDesktop != w.desktop() && w.desktop() != 0xFFFFFFFF)) {
                button->setFade(true);
            }
            if (active == w.WID()) {
                button->setCheckable(true);
                button->setChecked(true);
                connect(button, &FadeButton::toggled, [=]() {
                    button->setChecked(true);
                });
            }
            button->setIcon(w.icon());
            connect(button, SIGNAL(clicked(bool)), this, SLOT(ActivateWindow()));

            //If window is requesting attention, highlight it
            if (w.attention()) {
                button->setStyleSheet("background-color: #AAAA0000;");
            }

            //Add the button to the layout
            ui->windowList->layout()->addWidget(button);
        }

        ui->centralWidget->adjustSize();

        attentionDemandingWindows = demandAttention;
        this->repaint();
    }

    //Update all window geometries
    for (int i = 0; i < ui->windowList->layout()->count(); i++) {
        QWidget* widget = ((FlowLayout*) ui->windowList->layout())->itemAt(i)->widget();
        Window wid = widget->property("windowid").value<Window>();
        //Change the icon geometry of the window
        unsigned long* iconGeometry = (unsigned long*) malloc(sizeof(unsigned long) * 4);

        QPoint buttonScreenCoordinates = widget->mapToGlobal(QPoint(0, 0));
        if (buttonScreenCoordinates.y() < 0) buttonScreenCoordinates.setY(0);
        if (buttonScreenCoordinates.x() < 0) buttonScreenCoordinates.setX(0);
        iconGeometry[0] = buttonScreenCoordinates.x();
        iconGeometry[1] = buttonScreenCoordinates.y();
        iconGeometry[2] = widget->sizeHint().width();
        iconGeometry[3] = widget->sizeHint().height();
        XChangeProperty(QX11Info::display(), wid, XInternAtom(QX11Info::display(), "_NET_WM_ICON_GEOMETRY", False),
                        XA_CARDINAL, 32, PropModeReplace, (unsigned char*) iconGeometry, 4);

        free(iconGeometry);
    }

    oldDesktop = currentDesktop; //Keep the current desktop for tracking purposes
    oldActiveWindow = active;*/

    if (!lockHide && barAnim->state() != tPropertyAnimation::Running) { //Check for move lock
        int highestWindow, dockTop;
        if (settings.value("bar/onTop", true).toBool()) {
            if (settings.value("bar/statusBar", false).toBool()) {
                dockTop = screenGeometry.y() + 24 * getDPIScaling();
            } else {
                if (QTouchDevice::devices().count() > 0) {
                    //Leave a pixel to detect touch devices
                    dockTop = screenGeometry.y() + 1;
                } else {
                    dockTop = screenGeometry.y();
                }
            }

            highestWindow = screenGeometry.bottom();
            for (WmWindow window : taskbarManager->Windows()) {
                if (!window.isMinimized()) {
                    if (window.geometry().top() < highestWindow &&
                            window.geometry().right() > screenGeometry.left() &&
                            window.geometry().left() < screenGeometry.right()) {
                        highestWindow = window.geometry().top();
                    }
                }
            }
        } else {
            if (settings.value("bar/statusBar", false).toBool()) {
                dockTop = screenGeometry.bottom() - 24 * getDPIScaling();
            } else {
                dockTop = screenGeometry.bottom() + 1;
            }

            highestWindow = screenGeometry.top();
            for (WmWindow window : taskbarManager->Windows()) {
                if (!window.isMinimized()) {
                    if (window.geometry().bottom() > highestWindow &&
                            window.geometry().right() > screenGeometry.left() &&
                            window.geometry().left() < screenGeometry.right()) {
                        highestWindow = window.geometry().bottom();
                    }
                }
            }
        }

        bool doAnim = true;

        int finalTop;
        if (settings.value("bar/onTop", true).toBool()) {
            if (this->geometry().adjusted(0, 0, 0, 1).contains(QCursor::pos())) {
                if (settings.value("bar/statusBar").toBool() && !settings.value("bar/autoshow").toBool()) {
                    //Don't move bar; wait for click
                    doAnim = false;
                } else {
                    //Completely extend the bar
                    finalTop = screenGeometry.y();

                    //Hide the tutorial for the bar
                    TutorialWin->hideScreen(TutorialWindow::BarLocation);
                }
            } else {
                if (qMax((float) dockTop, highestWindow - 50 * getDPIScaling()) - this->height() > screenGeometry.y()) {
                    finalTop = screenGeometry.y();

                    //Hide the tutorial for the bar
                    TutorialWin->hideScreen(TutorialWindow::BarLocation);
                } else {
                    finalTop = qMax((float) dockTop, highestWindow - 50 * getDPIScaling()) - this->height();

                    //Show the tutorial for the bar
                    TutorialWin->showScreen(TutorialWindow::BarLocation);
                }
            }
        } else {
            if (this->geometry().adjusted(0, -1, 0, 0).contains(QCursor::pos())) {
                if (settings.value("bar/statusBar").toBool() && !settings.value("bar/autoshow").toBool()) {
                    //Don't move bar; wait for click
                    doAnim = false;
                } else {
                    //Completely extend the bar
                    finalTop = screenGeometry.bottom() - this->height() + 1;

                    //Hide the tutorial for the bar
                    TutorialWin->hideScreen(TutorialWindow::BarLocation);
                }
            } else {
                if (qMin((float) dockTop, highestWindow + 50 * getDPIScaling()) < screenGeometry.bottom() - this->height() + 1) {
                    finalTop = screenGeometry.bottom() - this->height() + 1;

                    //Hide the tutorial for the bar
                    TutorialWin->hideScreen(TutorialWindow::BarLocation);
                } else {
                    finalTop = qMin((float) dockTop, highestWindow + 50 * getDPIScaling());

                    //Show the tutorial for the bar
                    TutorialWin->showScreen(TutorialWindow::BarLocation);
                }
            }
        }

        if (doAnim) {
            if (finalTop == this->y()) {
                //barAnim->stop();
            } else {
                barAnim->setStartValue(this->geometry());
                barAnim->setEndValue(QRect(screenGeometry.x(), finalTop, screenGeometry.width(), this->height()));
                barAnim->start();
            }


            if (settings.value("bar/statusBar", false).toBool()) {
                if (finalTop == dockTop - this->height() || finalTop == dockTop) {
                    if (!statusBarVisible) {
                        ui->StatusBarFrame->setVisible(true);
                        tPropertyAnimation* statAnim = new tPropertyAnimation(statusBarOpacityEffect, "opacity");
                        statAnim->setStartValue((float) statusBarOpacityEffect->opacity());
                        statAnim->setEndValue((float) 1);
                        statAnim->setDuration(250);
                        connect(statAnim, SIGNAL(finished()), statAnim, SLOT(deleteLater()));
                        statAnim->start();
                        statusBarVisible = true;
                    }
                } else {
                    if (statusBarVisible) {
                        tPropertyAnimation* statAnim = new tPropertyAnimation(statusBarOpacityEffect, "opacity");
                        statAnim->setStartValue((float) statusBarOpacityEffect->opacity());
                        statAnim->setEndValue((float) 0);
                        statAnim->setDuration(250);
                        connect(statAnim, SIGNAL(finished()), statAnim, SLOT(deleteLater()));
                        connect(statAnim, &tPropertyAnimation::finished, [=]() {
                            ui->StatusBarFrame->setVisible(false);
                        });
                        statAnim->start();
                        statusBarVisible = false;
                    }
                }
            } else {
                ui->StatusBarFrame->setVisible(false);
            }
        } else {
            //barAnim->stop();
        }
    }

    if (settings.value("bar/onTop", true).toBool()) {
        ((QBoxLayout*) ui->centralWidget->layout())->setDirection(QBoxLayout::TopToBottom);
    } else {
        ((QBoxLayout*) ui->centralWidget->layout())->setDirection(QBoxLayout::BottomToTop);
    }
    forceWindowMove = false;

    //Update date and time
    if (settings.value("bar/compact", false).toBool()) {
        ui->date->setText(QLocale().toString(QDateTime::currentDateTime().date(), QLocale::ShortFormat /*"dd/mm/yy"*/));
    } else {
        ui->date->setText(QLocale().toString(QDateTime::currentDateTime(), "ddd dd MMM yyyy"));
    }

    if (settings.value("time/use24hour", true).toBool()) {
        ui->time->setText(QDateTime::currentDateTime().time().toString("HH:mm:ss"));
        ui->ampmLabel->setVisible(false);
    } else {
        QTime now = QDateTime::currentDateTime().time();
        if (QDateTime::currentDateTime().time().hour() < 12) {
            ui->ampmLabel->setText(QLocale().amText());
        } else {
            ui->ampmLabel->setText(QLocale().pmText());
            now = now.addSecs(-43200);
        }

        if (now.hour() == 0) now = now.addSecs(43200);

        ui->time->setText(now.toString("hh:mm:ss"));
        ui->ampmLabel->setVisible(true);
    }

    QString statusBarText = ui->time->text();
    if (!settings.value("time/use24hour", true).toBool()) {
        statusBarText.append(" " + ui->ampmLabel->text());
    }
    ui->StatusBarClock->setText(statusBarText);

    QStringList currentMprisApps;
    for (QString service : QDBusConnection::sessionBus().interface()->registeredServiceNames().value()) {
        if (service.startsWith("org.mpris.MediaPlayer2")) {
            currentMprisApps.append(service);
            if (!mprisDetectedApps.contains(service)) {
                DBusNewService(service);
            }
        }
    }
    mprisDetectedApps = currentMprisApps;

    if (!pauseMprisMenuUpdate) {
        if (mprisDetectedApps.count() > 1) {
            QMenu* menu = ui->mprisSelection->menu();
            menu->clear();
            for (QString app : mprisDetectedApps) {
                QAction* action = new QAction(nullptr);
                action->setData(app);
                action->setCheckable(true);
                if (mprisCurrentAppName == app) {
                    action->setChecked(true);
                }
                action->setText(app.remove("org.mpris.MediaPlayer2."));
                menu->addAction(action);
            }
            ui->mprisSelection->setVisible(true);
        } else {
            ui->mprisSelection->setVisible(false);
        }
    }

    if (mprisCurrentAppName != "") {
        ui->mprisFrame->setVisible(true);
        if (!mprisDetectedApps.contains(mprisCurrentAppName)) { //Service closed.
            if (mprisDetectedApps.count() > 0) { //Set to next app
                setMprisCurrentApp(mprisDetectedApps.first());
                ui->mprisFrame->setVisible(true);
                ui->StatusBarMpris->setVisible(true);
                ui->StatusBarMprisIcon->setVisible(true);
            } else { //Set to no app. Make mpris controller invisible.
                setMprisCurrentApp("");
                ui->mprisFrame->setVisible(false);
                ui->StatusBarMpris->setVisible(false);
                ui->StatusBarMprisIcon->setVisible(false);
            }
        }
    } else { //Make mpris controller invisible
        ui->mprisFrame->setVisible(false);
        ui->StatusBarMpris->setVisible(false);
        ui->StatusBarMprisIcon->setVisible(false);
    }
}

void MainWindow::setMprisCurrentApp(QString app) {
    if (mprisCurrentAppName != "") {
        QDBusConnection::sessionBus().disconnect(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateMpris(QString,QMap<QString, QVariant>,QStringList)));
    }
    mprisCurrentAppName = app;
    QDBusConnection::sessionBus().connect(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateMpris(QString,QMap<QString, QVariant>,QStringList)));
    updateMpris();
}

void MainWindow::lockMovement(QString reason) {
    lockHideCount++;
    lockHide = true;
    qDebug() << "Locking movement @" << lockHideCount << reason;
}

void MainWindow::unlockMovement(QString reason) {
    if (lockHideCount > 0) lockHideCount--;
    if (lockHideCount == 0) lockHide = false;
    qDebug() << "Unlocking movement @" << lockHideCount << reason;
}

void MainWindow::updateMpris() {
    if (!mprisUpdaterLocker.tryLock()) {
        return;
    }

    QEventLoop loop;

    //Get Current Song Metadata
    QDBusMessage MetadataRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    MetadataRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Metadata");

    QDBusPendingCallWatcher watcher(QDBusConnection::sessionBus().asyncCall(MetadataRequest, 100));
    connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &loop, SLOT(quit()));
    loop.exec();

    QVariantMap replyData;
    QDBusArgument arg(watcher.reply().arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>());

    arg >> replyData;

    QString title = "";
    QString artist = "";

    if (replyData.contains("xesam:title")) {
        title = replyData.value("xesam:title").toString();
    }
    this->mprisTitle = title;

    if (replyData.contains("xesam:artist")) {
        QStringList artists = replyData.value("xesam:artist").toStringList();
        for (QString art : artists) {
            artist.append(art + ", ");
        }
        artist.remove(artist.length() - 2, 2);
    }
    this->mprisArtist = artist;

    if (replyData.contains("xesam:album")) {
        this->mprisAlbum = replyData.value("xesam:album").toString();
    } else {
        this->mprisAlbum = "";
    }

    QString songName;
    if (title == "") {
        QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

        QDBusPendingCallWatcher watcher(QDBusConnection::sessionBus().asyncCall(IdentityRequest, 100));
        connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &loop, SLOT(quit()));
        loop.exec();

        //QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
        songName = watcher.reply().arguments().first().value<QDBusVariant>().variant().toString();
    } else {
        if (artist == "") {
            songName = title;
        } else {
            songName = artist + " · " + title;
        }
    }

    //Get Playback Status
    QDBusMessage PlayStatRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
    PlayStatRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "PlaybackStatus");

    QDBusPendingCallWatcher PlayStatWatcher(QDBusConnection::sessionBus().asyncCall(PlayStatRequest, 100));
    connect(&PlayStatWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &loop, SLOT(quit()));
    loop.exec();

    QDBusReply<QVariant> PlayStat(PlayStatWatcher.reply());
    if (PlayStat.value().toString() == "Playing") {
        ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
        ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
        mprisPlaying = true;
    } else {
        ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-pause").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
        mprisPlaying = false;
    }
    ui->mprisSongName->setText(songName);
    ui->StatusBarMpris->setText(songName);
    ui->StatusBarMpris->setVisible(true);
    ui->StatusBarMprisIcon->setVisible(true);
    ui->StatusBarFrame->setFixedWidth(this->width());

    mprisUpdaterLocker.unlock();
}

void MainWindow::on_time_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::ActivateWindow() {
    Window winId = sender()->property("windowid").value<Window>();


    Window activeWindow = 0;
    { //Get the active window
        Window *activeWin;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_ACTIVE_WINDOW", False), 0, 1024, False,
                                        AnyPropertyType, &ReturnType, &format, &items, &bytes, (unsigned char**) &activeWin);
        if (retval == 0 && activeWin != 0) {
             activeWindow = *activeWin;
        }
        XFree(activeWin);
    }

    if (winId == activeWindow) {
        //Minimise the window
        sendMessageToRootWindow("WM_CHANGE_STATE", winId, 3);
    } else {
        //Switch to the desktop that the window is in
        sendMessageToRootWindow("_NET_CURRENT_DESKTOP", 0, sender()->property("desktop").toInt());

        //Activate the window
        sendMessageToRootWindow("_NET_ACTIVE_WINDOW", winId, 2);
        XMapRaised(QX11Info::display(), winId);
    }
}

void MainWindow::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QMainWindow::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(this->sizeHint().height()));
    this->setFixedSize(w, this->sizeHint().height());
    ui->infoScrollArea->setFixedWidth(w - this->centralWidget()->layout()->margin());

    if (settings.value("bar/onTop", true).toBool()) {
        ui->StatusBarFrame->move(0, this->height() - 25 * getDPIScaling());
        seperatorWidget->setGeometry(0, this->height() - 1, this->width(), 1);
    } else {
        ui->StatusBarFrame->move(0, 1);
        seperatorWidget->setGeometry(0, 0, this->width(), 1);
    }
    statusBarNormalY = ui->StatusBarFrame->y();
    seperatorWidget->raise();
}

void MainWindow::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void MainWindow::on_date_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::on_batteryLabel_clicked()
{
    infoPane->show(InfoPaneDropdown::Battery);
}

void MainWindow::on_volumeFrame_MouseEnter()
{
    if (AudioMan->QuietMode() != AudioManager::mute) {
        ui->volumeSlider->setVisible(true);

        //Animate the volume frame
        tVariantAnimation* anim = new tVariantAnimation;
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->volumeSlider->setFixedWidth(value.toInt());
        });
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->setStartValue(ui->volumeSlider->width());
        anim->setEndValue((int) (150 * getDPIScaling()));
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();

        //Set Current Volume Slider
        ui->volumeSlider->setValue(AudioMan->MasterVolume());
        ui->volumeSlider->setMaximum(100);
    }
}

void MainWindow::on_volumeFrame_MouseExit()
{
    if (AudioMan->QuietMode() != AudioManager::mute) {
        //Animate the volume frame
        tVariantAnimation* anim = new tVariantAnimation;
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->volumeSlider->setFixedWidth(value.toInt());
        });
        connect(anim, &tVariantAnimation::finished, [=]() {
            ui->volumeSlider->setVisible(false);
            anim->deleteLater();
        });
        anim->setStartValue(ui->volumeSlider->width());
        anim->setEndValue(0);
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::InCubic);
        anim->start();
    }
}

void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    AudioMan->setMasterVolume(position);
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    on_volumeSlider_sliderMoved(value);
}

void MainWindow::on_brightnessFrame_MouseEnter()
{
    //Animate the brightness frame
    ui->brightnessSlider->setVisible(true);
    tVariantAnimation* anim = new tVariantAnimation;
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->brightnessSlider->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->setStartValue(ui->brightnessSlider->width());
    anim->setEndValue((int) (150 * getDPIScaling()));
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();

    //Get Current Brightness
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -get");
    backlight->waitForFinished();
    float output = ceil(QString(backlight->readAll()).toFloat());
    delete backlight;

    ui->brightnessSlider->setValue((int) output);
}

void MainWindow::on_brightnessFrame_MouseExit()
{

    tVariantAnimation* anim = new tVariantAnimation;
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->brightnessSlider->setFixedWidth(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, [=]() {
        ui->brightnessSlider->setVisible(false);
        anim->deleteLater();
    });
    anim->setStartValue(ui->brightnessSlider->width());
    anim->setEndValue(0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start();
}

void MainWindow::on_brightnessSlider_sliderMoved(int position)
{
    QProcess* backlight = new QProcess(this);
    backlight->start("xbacklight -set " + QString::number(position));
    connect(backlight, SIGNAL(finished(int)), backlight, SLOT(deleteLater()));
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    on_brightnessSlider_sliderMoved(value);
}

void MainWindow::on_volumeSlider_sliderReleased()
{
    //Check if the user has feedback sound on
    if (settings.value("sound/feedbackSound", true).toBool()) {
        QSoundEffect* volumeSound = new QSoundEffect();
        volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
        volumeSound->play();
        connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (painter.isActive()) {
        if (this->attentionDemandingWindows > 0) {
            if (!warningAnimCreated) {
                warningAnimCreated = true;
                tVariantAnimation* anim = new tVariantAnimation(this);
                anim->setStartValue(0);
                anim->setEndValue(this->width());
                anim->setEasingCurve(QEasingCurve::OutBounce);
                anim->setDuration(1000);
                anim->setForceAnimation(true);
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                connect(anim, &tVariantAnimation::valueChanged, [=](QVariant var) {
                    this->warningWidth = var.toInt();
                    this->repaint();
                });

                anim->start();
            }

            painter.setPen(QColor::fromRgb(255, 0, 0));
            int x1 = (this->width() / 2) - (this->warningWidth / 2);
            int x2 = (this->width() / 2) + (this->warningWidth / 2);
            painter.drawLine(x1, this->height() - 1, x2, this->height() - 1);
            painter.drawLine(x1, this->height() - 2, x2, this->height() - 2);
        } else {
            /*painter.setPen(this->palette().color(QPalette::WindowText));

            if (settings.value("bar/onTop", true).toBool()) {
                painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
            } else {
                painter.drawLine(0, 0, this->width(), 0);
            }*/
            warningAnimCreated = false;
        }
    }
    event->accept();
}

InfoPaneDropdown* MainWindow::getInfoPane() {
    return this->infoPane;
}

void MainWindow::setTimer(QString timer) {
    ui->timer->setText(timer);
    ui->timer->setVisible(true);
    ui->timerIcon->setVisible(true);
}

void MainWindow::setTimerVisible(bool visible) {
    ui->timer->setVisible(visible);
    ui->timerIcon->setVisible(visible);
}

void MainWindow::setTimerEnabled(bool enable) {
    ui->timer->setShowDisabled(!enable);
    ui->timerIcon->setShowDisabled(!enable);
}

void MainWindow::on_timerIcon_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::on_timer_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::on_mprisPause_clicked()
{
    playPause();
}

void MainWindow::on_mprisBack_clicked()
{
    previousSong();
}

void MainWindow::on_mprisForward_clicked()
{
    nextSong();
}

void MainWindow::on_mprisSongName_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Raise"), QDBus::NoBlock);
}

void MainWindow::reloadScreens() {
    forceWindowMove = true;
    updateStruts();
    ui->StatusBarFrame->setFixedWidth(this->width());
}

void MainWindow::show() {
    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_DOCK", False);
    int retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QMainWindow::show();

    updateStruts();

    //Show Gateway tutorial
    TutorialWin->showScreen(TutorialWindow::Gateway);
}

void MainWindow::on_desktopNext_clicked()
{
    int numOfDesktops = 0;
    {
        unsigned long *desktops;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktops);
        if (retval == 0 && desktops != 0) {
            numOfDesktops = *desktops;
        }
        XFree(desktops);
    }
    int switchToDesktop = ui->desktopName->property("desktopIndex").toInt() + 1;
    if (switchToDesktop == numOfDesktops) {
        switchToDesktop = 0;
    }

    sendMessageToRootWindow("_NET_CURRENT_DESKTOP", 0, switchToDesktop);
}

void MainWindow::on_desktopBack_clicked()
{
    int numOfDesktops = 0;
    {
        unsigned long *desktops;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_NUMBER_OF_DESKTOPS", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktops);
        if (retval == 0 && desktops != 0) {
            numOfDesktops = *desktops;
        }
        XFree(desktops);
    }
    int switchToDesktop = ui->desktopName->property("desktopIndex").toInt() - 1;
    if (switchToDesktop == -1) {
        switchToDesktop = numOfDesktops - 1;
    }

    sendMessageToRootWindow("_NET_CURRENT_DESKTOP", 0, switchToDesktop);
}

void MainWindow::openMenu() {
    lockMovement("Gateway");

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QRect availableGeometry = QApplication::desktop()->availableGeometry();

    int left;
    if (QApplication::isRightToLeft()) {
        left = this->x() + this->width();
    } else {
        left = this->x() - gatewayMenu->width();
    }

    //gatewayMenu->setGeometry(availableGeometry.x(), availableGeometry.y(), gatewayMenu->width(), availableGeometry.height());
    if (settings.value("bar/onTop", true).toBool()) {
        gatewayMenu->setGeometry(left, this->y() + this->height() - 1, gatewayMenu->width(), availableGeometry.height() - (this->height() + (this->y() - availableGeometry.y())) + 1);
    } else {
        int height;

        if (availableGeometry.bottom() < screenGeometry.height() - this->height()) {
            height = availableGeometry.height();
        } else {
            height = this->y() - screenGeometry.top() + 1;
        }
        gatewayMenu->setGeometry(left, availableGeometry.y() , gatewayMenu->width(), height);
    }
    gatewayMenu->show();
    gatewayMenu->setFocus();
}

bool MainWindow::isMprisAvailable() {
    return ui->mprisFrame->isVisible();
}

bool MainWindow::isMprisPlaying() {
    return this->mprisPlaying;
}

void MainWindow::nextSong() {
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Next"), QDBus::NoBlock);
}

void MainWindow::playPause() {
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "PlayPause"), QDBus::NoBlock);
}

void MainWindow::play() {
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Play"), QDBus::NoBlock);
}
void MainWindow::pause() {
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Pause"), QDBus::NoBlock);
}

void MainWindow::previousSong() {
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Previous"), QDBus::NoBlock);
}

QString MainWindow::mprisApp() {
    return this->mprisCurrentAppName;
}

QString MainWindow::songName() {
    return this->mprisTitle;
}

QString MainWindow::songArtist() {
    return this->mprisArtist;
}
QString MainWindow::songAlbum() {
    return this->mprisAlbum;
}

void MainWindow::on_mprisSelection_triggered(QAction *arg1)
{
    setMprisCurrentApp(arg1->data().toString());
}

void MainWindow::on_time_dragging(int x, int y)
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    infoPane->dragDown(InfoPaneDropdown::Clock, ui->time->mapToGlobal(QPoint(x, y)).y() - screenGeometry.top());
}

void MainWindow::on_time_mouseReleased()
{
    infoPane->completeDragDown();
}

void MainWindow::on_date_dragging(int x, int y)
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    infoPane->dragDown(InfoPaneDropdown::Clock, ui->time->mapToGlobal(QPoint(x, y)).y() - screenGeometry.top());
}

void MainWindow::on_date_mouseReleased()
{
    infoPane->completeDragDown();
}

void MainWindow::on_batteryLabel_dragging(int x, int y)
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    infoPane->dragDown(InfoPaneDropdown::Battery, ui->time->mapToGlobal(QPoint(x, y)).y() - screenGeometry.top());
}

void MainWindow::on_batteryLabel_mouseReleased()
{
    infoPane->completeDragDown();
}

void MainWindow::on_notifications_dragging(int x, int y)
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    infoPane->dragDown(InfoPaneDropdown::Clock, ui->time->mapToGlobal(QPoint(x, y)).y() - screenGeometry.top());
}

void MainWindow::on_notifications_mouseReleased()
{
    infoPane->completeDragDown();
}

void MainWindow::updateStruts() {
    long* struts = (long*) malloc(sizeof(long) * 12);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (settings.value("bar/statusBar", false).toBool()) {
        struts[0] = 0;
        struts[1] = 0;
        struts[4] = 0;
        struts[5] = 0;
        struts[6] = 0;
        struts[7] = 0;
        if (settings.value("bar/onTop", true).toBool()) {
            struts[2] = screenGeometry.top() + 24 * getDPIScaling();
            struts[3] = 0;
            struts[8] = screenGeometry.left();
            struts[9] = screenGeometry.right();
            struts[10] = 0;
            struts[11] = 0;
            ui->StatusBarHoverFrameDown->setPixmap(QIcon::fromTheme("go-down").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
            this->centralWidget()->layout()->setContentsMargins(0, 0, 0, 9);
            ui->InfoScrollWidget->layout()->setContentsMargins(9, 9, 9, 0);
        } else {
            struts[2] = 0;
            struts[3] = QApplication::desktop()->geometry().height() - screenGeometry.bottom() + 23 * getDPIScaling();
            struts[8] = 0;
            struts[9] = 0;
            struts[10] = screenGeometry.left();
            struts[11] = screenGeometry.right();
            ui->StatusBarHoverFrameDown->setPixmap(QIcon::fromTheme("go-up").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
            this->centralWidget()->layout()->setContentsMargins(0, 9, 0, 0);
            ui->InfoScrollWidget->layout()->setContentsMargins(9, 0, 9, 9);
        }
    } else {
        struts[0] = 0;
        struts[1] = 0;
        if (QTouchDevice::devices().count() > 0) {
            struts[2] = screenGeometry.top() + 1;
        } else {
            struts[2] = 0;
        }
        struts[3] = 0;
        struts[4] = 0;
        struts[5] = 0;
        struts[6] = 0;
        struts[7] = 0;
        struts[8] = 0;
        struts[9] = 0;
        struts[10] = 0;
        struts[11] = 0;
    }
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_STRUT_PARTIAL", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) struts, 12);

    free(struts);

    this->repaint();

    if (settings.value("bar/onTop", true).toBool()) {
        ui->StatusBarFrame->move(0, this->height() - 25 * getDPIScaling());
        ui->openStatusCenterButton->setIcon(QIcon::fromTheme("go-down"));
    } else {
        ui->StatusBarFrame->move(0, 1);
        ui->openStatusCenterButton->setIcon(QIcon::fromTheme("go-up"));
    }
}

void MainWindow::on_actionNone_triggered()
{
    AudioMan->setQuietMode(AudioManager::none);
}

void MainWindow::on_actionNotifications_triggered()
{
    AudioMan->setQuietMode(AudioManager::notifications);
}

void MainWindow::on_actionMute_triggered()
{
    AudioMan->setQuietMode(AudioManager::mute);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (watched == ui->StatusBarFrame || watched == ui->StatusBarHoverFrame || watched == ui->StatusBarProgressFrame) {
        if (event->type() == QEvent::MouseButtonPress) {
            gatewayMenu->close();
            //Completely extend the bar
            int finalTop;
            if (settings.value("bar/onTop", true).toBool()) {
                finalTop = screenGeometry.y();
            } else {
                finalTop = screenGeometry.bottom() - this->height() + 1;
            }

            barAnim->setStartValue(this->geometry());
            barAnim->setEndValue(QRect(screenGeometry.x(), finalTop, screenGeometry.width(), this->height()));
            barAnim->start();

            //Hide the tutorial for the bar
            TutorialWin->hideScreen(TutorialWindow::BarLocation);

            //Hide status bar
            if (settings.value("bar/statusBar", false).toBool() && statusBarVisible) {
                tPropertyAnimation* statAnim = new tPropertyAnimation(statusBarOpacityEffect, "opacity");
                statAnim->setStartValue((float) statusBarOpacityEffect->opacity());
                statAnim->setEndValue((float) 0);
                statAnim->setDuration(250);
                connect(statAnim, SIGNAL(finished()), statAnim, SLOT(deleteLater()));
                connect(statAnim, &tPropertyAnimation::finished, [=]() {
                    ui->StatusBarFrame->setVisible(false);

                    if (settings.value("bar/onTop", true).toBool()) {
                        ui->StatusBarHoverFrame->move(0, ui->StatusBarFrame->y() - ui->StatusBarFrame->height());
                    } else {
                        ui->StatusBarHoverFrame->move(0, ui->StatusBarFrame->y() + ui->StatusBarFrame->height());
                    }
                    ui->StatusBarHoverFrame->setVisible(false);
                });
                statAnim->start();

                statusBarVisible = false;
            }

            return true;
        } else if (event->type() == QEvent::Enter) {
            if (!settings.value("bar/autoshow").toBool()) {
                /*ui->StatusBarHoverFrame->setParent(ui->StatusBarFrame);
                ui->StatusBarHoverFrame->resize(ui->StatusBarFrame->size());
                ui->StatusBarHoverFrame->move(0, -ui->StatusBarHoverFrame->height());*/

                ui->StatusBarHoverFrame->setVisible(true);
                tPropertyAnimation* anim = new tPropertyAnimation(ui->StatusBarHoverFrame, "geometry");
                anim->setStartValue(ui->StatusBarHoverFrame->geometry());
                anim->setEndValue(QRect(0, ui->StatusBarFrame->y(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
                anim->setDuration(250);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                anim->start();
            }
            doUpdate();
            return true;
        } else if (event->type() == QEvent::Leave) {
            if (!settings.value("bar/autoshow").toBool() && statusBarVisible) {
                tPropertyAnimation* anim = new tPropertyAnimation(ui->StatusBarHoverFrame, "geometry");
                anim->setStartValue(ui->StatusBarHoverFrame->geometry());
                if (settings.value("bar/onTop", true).toBool()) {
                    anim->setEndValue(QRect(0, ui->StatusBarFrame->y() - ui->StatusBarFrame->height(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
                } else {
                    anim->setEndValue(QRect(0, ui->StatusBarFrame->y() + ui->StatusBarFrame->height(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
                }
                anim->setDuration(250);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                connect(anim, &tPropertyAnimation::finished, [=] {
                    ui->StatusBarHoverFrame->setVisible(false);
                });
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                anim->start();
            }
            return true;
        } else if ((watched == ui->StatusBarFrame || watched == ui->StatusBarProgressFrame) && event->type() == QEvent::Paint) {
            QPainter painter((QWidget*) watched);
            //Draw progress bar
            painter.setPen(Qt::transparent);
            painter.setBrush(this->palette().color(QPalette::Highlight));

            QRect rect;
            rect.setTopLeft(QPoint(0, 0));
            rect.setHeight(ui->StatusBarFrame->height());
            rect.setWidth(ui->StatusBarFrame->width() * ((float) statusBarPercentage / (float) 100));
            painter.drawRect(rect);
        }
    } else if (watched == seperatorWidget) {
        if (event->type() == QEvent::Paint) {
            QPainter painter(seperatorWidget);
            painter.setPen(this->palette().color(QPalette::WindowText));
            painter.drawLine(0, 0, seperatorWidget->width(), 0);
        }
    }
    return false;
}

void MainWindow::enterEvent(QEvent *event) {
    doUpdate();
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::on_stopRecordingButton_clicked()
{
    screenRecorder->stop();
}

void MainWindow::on_MainWindow_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu();
    menu->addSection(tr("For Bar"));
    if (settings.value("bar/onTop").toBool()) {
        menu->addAction(QIcon::fromTheme("go-down"), tr("Move to bottom"), [=] {
            settings.setValue("bar/onTop", false);
            infoPane->updateStruts();
        });
    } else {
        menu->addAction(QIcon::fromTheme("go-up"), tr("Move to top"), [=] {
            settings.setValue("bar/onTop", true);
            infoPane->updateStruts();
        });
    }

    menu->addAction(QIcon::fromTheme("configure"), tr("Gateway and Bar Settings"), [=] {
        getInfoPane()->show(InfoPaneDropdown::Settings);
        getInfoPane()->changeSettingsPane(1);
    });

    menu->addSection(tr("For System"));
    menu->addAction(QIcon::fromTheme("dialog-information"), tr("Open Status Center"), [=] {
        getInfoPane()->show(InfoPaneDropdown::Clock);
    });
    menu->addAction(QIcon::fromTheme("configure"), tr("Open System Settings"), [=] {
        getInfoPane()->show(InfoPaneDropdown::Settings);
    });
    menu->exec(this->mapToGlobal(pos));
}

void MainWindow::on_openMenu_customContextMenuRequested(const QPoint &pos)
{
    QMenu* menu = new QMenu();
    menu->addAction(ui->openMenu->icon(), tr("Open Gateway"), this, SLOT(openMenu()));
    menu->exec(ui->openMenu->mapToGlobal(pos));
}

void MainWindow::reloadBar() {
    ui->lowerBarLayout->setParent(nullptr);
    if (settings.value("bar/compact", false).toBool()) {
        ui->topBarLayout->insertLayout(6, ui->lowerBarLayout);
        ui->openStatusCenterButton->setVisible(true);
        ui->openMenu->setVisible(false);
        ui->openMenuCompact->setVisible(true);
        ui->dateLayout->setDirection(QBoxLayout::TopToBottom);
        ui->dateLayout->setSpacing(0);

        QBoxLayout* flow = new QBoxLayout(QBoxLayout::LeftToRight);
        flow->setSpacing(0);
        flow->setMargin(-1);
        ui->windowList->setLayout(flow);
    } else {
        ((QBoxLayout*) ui->centralWidget->layout())->insertLayout(1, ui->lowerBarLayout);
        ui->openStatusCenterButton->setVisible(false);
        ui->openMenu->setVisible(true);
        ui->openMenuCompact->setVisible(false);
        ui->dateLayout->setDirection(QBoxLayout::LeftToRight);
        ui->dateLayout->setSpacing(6);

        FlowLayout* flow = new FlowLayout(ui->windowList, -1, 0, 0);
        ui->windowList->setLayout(flow);
    }
}

void MainWindow::on_openStatusCenterButton_clicked()
{
    getInfoPane()->show(InfoPaneDropdown::Clock);
}

void MainWindow::on_actionCriticalOnly_triggered()
{
    AudioMan->setQuietMode(AudioManager::critical);
}

bool MainWindow::event(QEvent* event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (event->type() == QEvent::TouchBegin) {
        QTouchEvent* e = (QTouchEvent*) event;
        QPoint p = e->touchPoints().first().pos().toPoint();
        lastTouchPoint = this->geometry().topLeft();
        lastTouchScreenPoint = mapToGlobal(p);

        if (e->touchPoints().count() == 1) {
            if (this->geometry().top() == screenGeometry.top()) {
                if (p.y() == 0) {
                    currentTouch = 1; //Drag down Status Center
                    lockMovement("Touch Status Center");
                    e->accept();
                    return true;
                }
            } else {
                currentTouch = 0; //Move Info Pane
                lockMovement("Touch Bar");
                barAnim->pause(); //Stop bar animation
                e->accept();
                return true;
            }
        }
    } else if (event->type() == QEvent::TouchUpdate) {
        QTouchEvent* e = (QTouchEvent*) event;
        if (currentTouch == 0) { //Move Info Pane
            QPoint p = e->touchPoints().first().pos().toPoint();
            QPoint screenPoint = mapToGlobal(p);
            QPoint moveTo = (screenPoint - lastTouchScreenPoint) + lastTouchPoint;

            int top = moveTo.y();
            if (top > screenGeometry.top()) top = screenGeometry.top();
            if (settings.value("bar/statusBar", false).toBool()) {
                if (top < screenGeometry.top() - this->height() + 24 * getDPIScaling()) top = screenGeometry.top() - this->height() + 24 * getDPIScaling();
            } else {
                if (top < screenGeometry.top() - this->height() + 1) top = screenGeometry.top() - this->height() + 1;
            }
            this->move(screenGeometry.left(), top);
        } else if (currentTouch == 1) {
            infoPane->dragDown(InfoPaneDropdown::Clock, mapToGlobal(e->touchPoints().first().pos().toPoint()).y() - screenGeometry.top());
        }
    } else if (event->type() == QEvent::TouchEnd) {
        QTouchEvent* e = (QTouchEvent*) event;
        if (currentTouch == 0) {
            currentTouch = -1;
            QTimer::singleShot(1000, [=] {
                unlockMovement("Touch Bar");
            });
        } else if (currentTouch == 1) {
            currentTouch = -1;
            infoPane->completeDragDown();
            unlockMovement("Touch Status Center");
        }

    } else if (event->type() == QEvent::TouchCancel) {
        unlockMovement("Touch Cancel");
    }
    return QMainWindow::event(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    ui->StatusBarFrame->setFixedWidth(this->width());
}

void MainWindow::showStatusBarProgress(bool show) {
    if (show) {
        if (ui->StatusBarFrame->isVisible() && !ui->StatusBarProgressFrame->isVisible()) {
            ui->StatusBarProgressFrame->setVisible(true);

            tPropertyAnimation* anim = new tPropertyAnimation(ui->StatusBarProgressFrame, "geometry");
            anim->setStartValue(ui->StatusBarProgressFrame->geometry());
            anim->setEndValue(QRect(0, ui->StatusBarFrame->y(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
            anim->setDuration(250);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &tPropertyAnimation::valueChanged, [=](QVariant value) {
                if (settings.value("bar/onTop", true).toBool()) {
                    ui->StatusBarFrame->move(0, value.toRect().bottom());
                } else {
                    ui->StatusBarFrame->move(0, value.toRect().top() - ui->StatusBarFrame->height());
                }
            });
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();
        }
    } else {
        if (ui->StatusBarProgressFrame->isVisible()) {
            tPropertyAnimation* anim = new tPropertyAnimation(ui->StatusBarProgressFrame, "geometry");
            anim->setStartValue(ui->StatusBarProgressFrame->geometry());
            if (settings.value("bar/onTop", true).toBool()) {
                anim->setEndValue(QRect(0, statusBarNormalY - ui->StatusBarFrame->height() + 1, ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
            } else {
                anim->setEndValue(QRect(0, statusBarNormalY + ui->StatusBarFrame->height(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
            }
            anim->setDuration(250);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &tPropertyAnimation::valueChanged, [=](QVariant value) {
                if (settings.value("bar/onTop", true).toBool()) {
                    ui->StatusBarFrame->move(0, value.toRect().bottom());
                } else {
                    ui->StatusBarFrame->move(0, value.toRect().top() - ui->StatusBarFrame->height());
                }
            });
            connect(anim, &tPropertyAnimation::finished, [=] {
                ui->StatusBarProgressFrame->setVisible(false);
            });
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();
        }
    }
}
