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

#include <QScroller>
#include <mpris/mprisengine.h>
#include <mpris/mprisplayer.h>
#include <globalkeyboard/globalkeyboardengine.h>
#include "soundengine.h"

#include "menu.h"
#include "infopanedropdown.h"
#include "powerdaemon.h"

#include <Wm/desktopwm.h>
#include <TimeDate/desktoptimedate.h>
#include <quietmodedaemon.h>

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);
extern DbusEvents* DBusEvents;
extern TutorialWindow* TutorialWin;
extern AudioManager* AudioMan;
extern NativeEventFilter* NativeFilter;
extern float getDPIScaling();
extern LocationServices* locationServices;
extern ScreenRecorder* screenRecorder;

struct MainWindowPrivate {
    tVariantAnimation* barAnim;
    bool hasMouse = false;

    bool isHidden = false;

    QGraphicsOpacityEffect* statusBarOpacityEffect;
    tVariantAnimation* statusBarOpacityAnimation;
    bool statusBarVisible = false;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    d = new MainWindowPrivate();

    this->setAttribute(Qt::WA_AcceptTouchEvents, true);
    ui->StatusBarFrame->setAttribute(Qt::WA_AcceptTouchEvents, true);
    this->setMouseTracking(true);

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
    QActionGroup* mprisGroup = new QActionGroup(this);
    QMenu* mprisSelectionMenu = new QMenu();
    mprisSelectionMenu->addSection(tr("Media Player"));
    ui->mprisSelection->setMenu(mprisSelectionMenu);
    connect(mprisSelectionMenu, &QMenu::aboutToShow, [=]() {
        lockMovement("MPRIS");
    });
    connect(mprisSelectionMenu, &QMenu::aboutToHide, [=]() {
        unlockMovement("MPRIS");
    });

    //Prepare MPRIS
    setMprisCurrentApp(nullptr);
    auto addMprisPlayer = [=](QString service, MprisPlayerPtr player) {
        Q_UNUSED(service)
        QAction* action = mprisSelectionMenu->addAction(player->identity());
        action->setData(QVariant::fromValue(player));
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=] {
            setMprisCurrentApp(action);
        });
        connect(player.data(), &MprisPlayer::identityChanged, action, [=] {
            action->setText(player->identity());
        });
        connect(player.data(), &MprisPlayer::gone, action, &QAction::deleteLater);
        mprisGroup->addAction(action);

        if (currentPlayer.isNull()) {
            setMprisCurrentApp(action);
        }
    };
    for (MprisPlayerPtr player : MprisEngine::players()) {
        addMprisPlayer(player->service(), player);
    }
    connect(MprisEngine::instance(), &MprisEngine::newPlayer, this, addMprisPlayer);

    //Connect signals related to multiple monitor management
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));

    remakeBar();

    //Set up bar movement
    d->barAnim = new tPropertyAnimation(this, "geometry");
    d->barAnim->setDuration(500);
    d->barAnim->setEasingCurve(QEasingCurve::OutCubic);

    //Create the gateway and set required flags
    gatewayMenu = new Menu(this);
    gatewayMenu->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(gatewayMenu, &Menu::menuClosing, [=]() {
        unlockMovement("Gateway closing");
    });

    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    reloadBar();

    //Create the update event timer and start it
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, &MainWindow::doUpdate);
    timer->start();

    this->doUpdate();

    //Prepare the time labels
    DesktopTimeDate::makeTimeLabel(ui->time, DesktopTimeDate::Time);
    DesktopTimeDate::makeTimeLabel(ui->ampmLabel, DesktopTimeDate::AmPm);
    DesktopTimeDate::makeTimeLabel(ui->StatusBarClock, DesktopTimeDate::Time);
    DesktopTimeDate::makeTimeLabel(ui->StatusBarClockAmPm, DesktopTimeDate::AmPm);

    connect(PowerDaemon::instance(), &PowerDaemon::powerStretchChanged, this, [=](bool isOn) {
        if (isOn) {
            timer->setInterval(1000);
        } else {
            timer->setInterval(100);
        }
    });
    if (PowerDaemon::instance()->powerStretch()) {
        timer->setInterval(1000);
    } else {
        timer->setInterval(100);
    }


    infoPane = new InfoPaneDropdown(this->winId());
    connect(infoPane, SIGNAL(timerEnabledChanged(bool)), this, SLOT(setTimerEnabled(bool)));
    connect(infoPane, SIGNAL(updateStrutsSignal()), this, SLOT(updateStruts()));
    connect(infoPane, SIGNAL(updateBarSignal()), this, SLOT(reloadBar()));
    connect(infoPane, &InfoPaneDropdown::flightModeChanged, [=](bool flight) {
        ui->StatusBarFlight->setVisible(flight);
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
    ui->keyboardButton->setVisible(false);

    connect(ui->openMenuCompact, SIGNAL(customContextMenuRequested(QPoint)), ui->openMenu, SIGNAL(customContextMenuRequested(QPoint)));

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
    ui->StatusBarQuietMode->setVisible(false);
    ui->StatusBarLocation->setVisible(false);
    ui->LocationIndication->setVisible(false);
    ui->StatusBarRecording->setVisible(false);
    ui->StatusBarLocation->setPixmap(QIcon::fromTheme("gps").pixmap(ic16));
    ui->LocationIndication->setPixmap(QIcon::fromTheme("gps").pixmap(ic16));
    ui->StatusBarFlight->setPixmap(QIcon::fromTheme("flight-mode").pixmap(ic16));
    ui->StatusBarFlight->setVisible(settings.value("flightmode/on", false).toBool());
    ui->StatusBarFrame->installEventFilter(this);
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

    d->statusBarOpacityEffect = new QGraphicsOpacityEffect();
    d->statusBarOpacityEffect->setOpacity(0);
    ui->StatusBarFrame->setGraphicsEffect(d->statusBarOpacityEffect);

    d->statusBarOpacityAnimation = new tVariantAnimation();
    d->statusBarOpacityAnimation->setStartValue(0.0);
    d->statusBarOpacityAnimation->setEndValue(1.0);
    d->statusBarOpacityAnimation->setDuration(500);
    d->statusBarOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->statusBarOpacityAnimation, &tVariantAnimation::valueChanged, this, [=](QVariant value) {
        if (qFuzzyCompare(value.toReal(), 1)) {
            d->statusBarOpacityEffect->setEnabled(false);
        } else {
            d->statusBarOpacityEffect->setEnabled(true);
            d->statusBarOpacityEffect->setOpacity(value.toReal());
        }

        ui->StatusBarFrame->setVisible(!qFuzzyIsNull(d->statusBarOpacityEffect->opacity()));
    });

    connect(locationServices, &LocationServices::locationUsingChanged, [=](bool location) {
        ui->StatusBarLocation->setVisible(location);
        ui->LocationIndication->setVisible(location);
    });

    ui->volumeSlider->setVisible(false);
    ui->brightnessSlider->setVisible(false);

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

    connect(QuietModeDaemon::instance(), &QuietModeDaemon::QuietModeChanged, [=](QuietModeDaemon::QuietMode newMode) {
        switch (newMode) {
            case QuietModeDaemon::None:
                ui->volumeButton->setIcon(QIcon::fromTheme("audio-volume-high"));
                ui->StatusBarQuietMode->setVisible(false);
                break;
            case QuietModeDaemon::Critical:
                ui->volumeButton->setIcon(QIcon::fromTheme("quiet-mode-critical-only"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("quiet-mode-critical-only").pixmap(SC_DPI_T(QSize(16, 16), QSize)));
                ui->StatusBarQuietMode->setVisible(true);
                break;
            case QuietModeDaemon::Notifications:
                ui->volumeButton->setIcon(QIcon::fromTheme("quiet-mode"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("quiet-mode").pixmap(SC_DPI_T(QSize(16, 16), QSize)));
                ui->StatusBarQuietMode->setVisible(true);
                break;
            case QuietModeDaemon::Mute:
                ui->volumeButton->setIcon(QIcon::fromTheme("audio-volume-muted"));
                ui->StatusBarQuietMode->setPixmap(QIcon::fromTheme("audio-volume-muted").pixmap(SC_DPI_T(QSize(16, 16), QSize)));
                ui->StatusBarQuietMode->setVisible(true);
                break;
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

            ui->screenRecordingProcessingSpinner->setFixedWidth(SC_DPI(16));
        } else {
            ui->StatusBarRecording->setVisible(false);
            ui->screenRecordingFrame->setVisible(false);
        }
    });

    ui->StatusBarHoverFrame->setVisible(false);
    ui->StatusBarProgressFrame->setVisible(false);

    statusBarProgressTimer = new QTimer();
    statusBarProgressTimer->setInterval(5000);
    connect(statusBarProgressTimer, &QTimer::timeout, [=] {
        this->showStatusBarProgress(!ui->StatusBarProgressFrame->isVisible());
        if (statusBarPercentage == -2) statusBarProgressTimer->stop();
    });
    ui->StatusBarProgressSpinner->setFixedSize(QSize(16, 16) * getDPIScaling());

    QScroller::grabGesture(ui->infoScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    connect(GlobalKeyboardEngine::instance(), &GlobalKeyboardEngine::keyShortcutRegistered, this, [=](QString name, GlobalKeyboardKey* key) {
        if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::OpenGateway)) {
            connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
                this->openMenu();
            });
        }
    });

    initTaskbar();
    calculateAndMoveBar();

    //ui->infoScrollArea->setFixedHeight(ui->InfoScrollWidget->height());

    #ifdef BLUEPRINT
        //Apply Blueprint branding
        ui->openMenu->setIcon(QIcon(":/icons/icon-bp.svg"));
        ui->openMenuCompact->setIcon(QIcon(":/icons/icon-bp.svg"));
    #endif
}

MainWindow::~MainWindow()
{
    delete d;
    delete ui;
}

void MainWindow::remakeBar() {
    //This is a hack...
    qDebug() << "barAnim was destroyed :(";
    d->barAnim = new tVariantAnimation();
    d->barAnim->setDuration(500);
    d->barAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(d->barAnim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setGeometry(value.toRect());
    });
    connect(d->barAnim, &tVariantAnimation::destroyed, [=] {
        remakeBar();
    });
}

void MainWindow::pullDownGesture() {
    if (lockHide) {
        on_date_clicked();
    } else {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        d->barAnim->setStartValue(this->geometry());
        d->barAnim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width() + 1, this->height()));
        d->barAnim->start();

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
    static_cast<QBoxLayout*>(ui->centralWidget->layout())->setDirection(settings.value("bar/onTop", true).toBool() ? QBoxLayout::TopToBottom : QBoxLayout::BottomToTop);

    //Update date and time
    if (settings.value("bar/compact", false).toBool()) {
        ui->date->setText(QLocale().toString(QDateTime::currentDateTime().date(), QLocale::ShortFormat /*"dd/mm/yy"*/));
    } else {
        ui->date->setText(DesktopTimeDate::timeString(DesktopTimeDate::StandardDate));
    }
}

void MainWindow::setMprisCurrentApp(QAction* app) {
    //Set up the context object
    if (mprisContextObject != nullptr) mprisContextObject->deleteLater();
    mprisContextObject = new QObject(this);

    if (app == nullptr) {
        ui->mprisFrame->setVisible(false);
        ui->StatusBarMpris->setVisible(false);
        ui->StatusBarMprisIcon->setVisible(false);
        currentPlayer = nullptr;
    } else {
        ui->mprisFrame->setVisible(true);
        ui->StatusBarMpris->setVisible(true);
        ui->StatusBarMprisIcon->setVisible(true);
        app->setChecked(true);

        //Connect signals
        currentPlayer = app->data().value<MprisPlayerPtr>();
        auto setMetadataFunction = [=] {
            QString statusString;
            QString title = currentPlayer->metadata().value("xesam:title").toString();
            if (title == "") {
                statusString = currentPlayer->identity();
            } else {
                QStringList parts;
                QString part2 = currentPlayer->metadata().value("xesam:artist").toStringList().join(", ");
                if (part2 == "") part2 = currentPlayer->metadata().value("xesam:album").toString();
                if (part2 != "") parts.append(part2);
                parts.append(title);

                statusString = parts.join(" · ");
            }

            ui->mprisSongName->setText(statusString);
            ui->StatusBarMpris->setText(statusString);
        };
        setMetadataFunction();
        if (currentPlayer->playbackStatus() == MprisPlayer::Playing) {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
            ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(SC_DPI(16), SC_DPI(16)));
        } else {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));
            ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-pause").pixmap(SC_DPI(16), SC_DPI(16)));
        }
        connect(currentPlayer.data(), &MprisPlayer::metadataChanged, mprisContextObject, setMetadataFunction);
        connect(currentPlayer.data(), &MprisPlayer::playbackStatusChanged, mprisContextObject, [=] {
            if (currentPlayer->playbackStatus() == MprisPlayer::Playing) {
                ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
                ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(SC_DPI(16), SC_DPI(16)));
            } else {
                ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));
                ui->StatusBarMprisIcon->setPixmap(QIcon::fromTheme("media-playback-pause").pixmap(SC_DPI(16), SC_DPI(16)));
            }
        });
        connect(currentPlayer.data(), &MprisPlayer::gone, mprisContextObject, [=] {
            QList<QAction*> actions = ui->mprisSelection->menu()->actions();
            actions.removeFirst(); //Remove the section heading
            actions.removeAll(app); //Remove the current player

            if (actions.count() > 0) {
                setMprisCurrentApp(actions.first());
            } else {
                setMprisCurrentApp(nullptr);
            }
        });
    }
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

void MainWindow::on_time_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
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

void MainWindow::on_volumeFrame_MouseEnter()
{
    if (QuietModeDaemon::getQuietMode() != QuietModeDaemon::Mute) {
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
    if (QuietModeDaemon::getQuietMode() != QuietModeDaemon::Mute) {
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
    //Play the audio change sound
    SoundEngine::play(SoundEngine::Volume);
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

Menu* MainWindow::getMenu() {
    return this->gatewayMenu;
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
    currentPlayer->playPause();
}

void MainWindow::addWindow(DesktopWmWindowPtr window)
{
    TaskbarButton* button = new TaskbarButton(window);

    //Add the button to the layout
    ui->windowList->layout()->addWidget(button);

    connect(window, &DesktopWmWindow::geometryChanged, this, &MainWindow::calculateAndMoveBar);
    calculateAndMoveBar();
}

void MainWindow::calculateAndMoveBar()
{
    QRect screenGeometry = QApplication::screens().first()->geometry();

    bool onTop = settings.value("bar/onTop", true).toBool();
    bool haveStatusBar = settings.value("bar/statusBar", false).toBool();

    ui->StatusBarFrame->setVisible(haveStatusBar && !qFuzzyIsNull(d->statusBarOpacityEffect->opacity()));

    bool shouldHide;
    if (d->hasMouse) {
        shouldHide = false;
    } else {
        //Get a QRect representing the windows on this desktop
        QRect rect;
        for (DesktopWmWindowPtr window : DesktopWm::openWindows()) {
            if (window->shouldShowInTaskbar() && window->isOnCurrentDesktop()) {
                QRect windowGeometry = window->geometry();

                //Ensure this window draws on this screen
                if (windowGeometry.intersects(screenGeometry)) {
                    rect = rect.united(window->geometry());
                }
            }
        }

        if (onTop) {
            shouldHide = (rect.top() < screenGeometry.y() + this->sizeHint().height());
        } else {
            shouldHide = (rect.bottom() > screenGeometry.bottom() - this->sizeHint().height());
        }
    }

    QRect endGeom;
    if (shouldHide) {
        //Figure out how much of the Bar should protrude
        int hideHeight = 1;
        if (haveStatusBar) hideHeight = ui->StatusBarFrame->height() + 1;

        if (onTop) {
            //Hide the bar on top
            endGeom = QRect(screenGeometry.x(), screenGeometry.y() - this->sizeHint().height() + hideHeight, screenGeometry.width(), this->sizeHint().height());
        } else {
            //Hide the bar on the bottom
            endGeom = QRect(screenGeometry.x(), screenGeometry.bottom() - hideHeight, this->width(), this->sizeHint().height());
        }

        if (haveStatusBar) {
            //Show the status bar
            if (d->statusBarOpacityAnimation->currentTime() != 500) {
                d->statusBarOpacityAnimation->setDirection(tVariantAnimation::Forward);
                d->statusBarOpacityAnimation->start();
            }
        }
    } else {
        if (onTop) {
            //Show the entirety of the bar on top
            endGeom = QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->sizeHint().height());
        } else {
            //Show the entirety of the bar on the bottom
            endGeom = QRect(screenGeometry.x(), screenGeometry.bottom() - this->sizeHint().height(), screenGeometry.width(), this->sizeHint().height());
        }

        if (haveStatusBar) {
            //Hide the status bar
            if (d->statusBarOpacityAnimation->currentTime() != 0) {
                d->statusBarOpacityAnimation->setDirection(tVariantAnimation::Backward);
                d->statusBarOpacityAnimation->start();
            }
        }
    }

    if (d->barAnim->endValue() == endGeom) return; //Nothing more needs to be done

    if (d->barAnim->state() == tVariantAnimation::Running) d->barAnim->stop();
    d->barAnim->setStartValue(this->geometry());
    d->barAnim->setEndValue(endGeom);
    d->barAnim->start();
    d->isHidden = shouldHide;
}

void MainWindow::on_mprisBack_clicked()
{
    currentPlayer->previous();
}

void MainWindow::on_mprisForward_clicked()
{
    currentPlayer->next();
}

void MainWindow::on_mprisSongName_clicked()
{
    currentPlayer->raise();
}

void MainWindow::reloadScreens() {
    calculateAndMoveBar();
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
    uint switchToDesktop = DesktopWm::currentDesktop() + 1;
    if (switchToDesktop == static_cast<uint>(DesktopWm::desktops().count())) {
        switchToDesktop = 0;
    }
    DesktopWm::setCurrentDesktop(switchToDesktop);
}

void MainWindow::on_desktopBack_clicked()
{
    uint numOfDesktops = static_cast<uint>(DesktopWm::desktops().count());
    uint switchToDesktop = DesktopWm::currentDesktop() - 1;
    if (switchToDesktop == UINT_MAX) {
        switchToDesktop = numOfDesktops - 1;
    }
    DesktopWm::setCurrentDesktop(switchToDesktop);
}

void MainWindow::openMenu() {
    if (!gatewayMenu->isVisible()) lockMovement("Gateway");

    //gatewayMenu->setGeometry(availableGeometry.x(), availableGeometry.y(), gatewayMenu->width(), availableGeometry.height());
    gatewayMenu->prepareForShow();
    gatewayMenu->show();
    gatewayMenu->setFocus();
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
        struts[2] = screenGeometry.top() + 1;
        struts[3] = 0;
        struts[4] = 0;
        struts[5] = 0;
        struts[6] = 0;
        struts[7] = 0;
        struts[8] = screenGeometry.left();
        struts[9] = screenGeometry.right();
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
    QuietModeDaemon::setQuietMode(QuietModeDaemon::None);
}

void MainWindow::on_actionNotifications_triggered()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Notifications);
}

void MainWindow::on_actionMute_triggered()
{
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Mute);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->StatusBarFrame || watched == ui->StatusBarHoverFrame || watched == ui->StatusBarProgressFrame) {
        if (event->type() == QEvent::MouseButtonPress) {
            gatewayMenu->close();

            //Completely extend the bar
            d->hasMouse = true;
            calculateAndMoveBar();

            //Hide the tutorial for the bar
            TutorialWin->hideScreen(TutorialWindow::BarLocation);
            return true;
        } else if ((watched == ui->StatusBarFrame || watched == ui->StatusBarProgressFrame) && event->type() == QEvent::Paint) {
            QPainter painter(static_cast<QWidget*>(watched));
            //Draw progress bar
            painter.setPen(Qt::transparent);
            painter.setBrush(this->palette().color(QPalette::Highlight));

            QRect rect;
            rect.setTopLeft(QPoint(0, 0));
            rect.setHeight(ui->StatusBarFrame->height());
            rect.setWidth(static_cast<int>(ui->StatusBarFrame->width() * (statusBarPercentage / 100.0)));
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
    if (settings.value("bar/autoshow").toBool()) {
        d->hasMouse = true;
    } else {
        ui->StatusBarHoverFrame->setVisible(true);
        tPropertyAnimation* anim = new tPropertyAnimation(ui->StatusBarHoverFrame, "geometry");
        anim->setStartValue(ui->StatusBarHoverFrame->geometry());
        anim->setEndValue(QRect(0, ui->StatusBarFrame->y(), ui->StatusBarFrame->width(), ui->StatusBarFrame->height()));
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tPropertyAnimation::finished, anim, &tPropertyAnimation::deleteLater);
        anim->start();
    }
    calculateAndMoveBar();
}

void MainWindow::leaveEvent(QEvent*event)
{
    if (!settings.value("bar/autoshow").toBool()) {
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
        connect(anim, &tPropertyAnimation::finished, anim, &tPropertyAnimation::deleteLater);
        anim->start();
    }

    d->hasMouse = false;
    calculateAndMoveBar();
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
        static_cast<QBoxLayout*>(ui->centralWidget->layout())->insertLayout(1, ui->lowerBarLayout);
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
    QuietModeDaemon::setQuietMode(QuietModeDaemon::Critical);
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
                d->barAnim->pause(); //Stop bar animation
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

void MainWindow::initTaskbar()
{
    connect(DesktopWm::instance(), &DesktopWm::currentDesktopChanged, this, [=] {
        ui->desktopName->setText(DesktopWm::desktops().at(static_cast<int>(DesktopWm::currentDesktop())));
    });
    connect(DesktopWm::instance(), &DesktopWm::desktopCountChanged, this, [=] {
        QStringList desktops = DesktopWm::desktops();
        ui->desktopsFrame->setVisible(desktops.count() > 1);
        if (ui->desktopsFrame->isVisible()) {
            ui->desktopName->setText(desktops.at(static_cast<int>(DesktopWm::currentDesktop())));
        }
    });
    connect(DesktopWm::instance(), &DesktopWm::windowAdded, this, &MainWindow::addWindow);

    QStringList desktops = DesktopWm::desktops();
    ui->desktopsFrame->setVisible(desktops.count() > 1);
    if (ui->desktopsFrame->isVisible()) {
        ui->desktopName->setText(desktops.at(static_cast<int>(DesktopWm::currentDesktop())));
    }

    for (DesktopWmWindowPtr window : DesktopWm::openWindows()) {
        addWindow(window);
    }
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
