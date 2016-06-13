#include "mainwindow.h"
#include "ui_mainwindow.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    ui->pushButton_4->setVisible(false);

    FlowLayout* flow = new FlowLayout(ui->windowList);
    ui->windowList->setLayout(flow);

    windowList = new QList<WmWindow*>();

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(reloadWindows()));
    timer->start();

    NotificationDBus* ndbus = new NotificationDBus(this);
    connect(ndbus, SIGNAL(newNotification(int,QString,QString,QIcon)), this, SLOT(incrementNotificationCounter()));
    connect(ndbus, SIGNAL(removeNotification(int)), this, SLOT(decrementNotificationCounter()));

    UPowerDBus* updbus = new UPowerDBus(ndbus, this);
    connect(updbus, &UPowerDBus::updateDisplay, [=](QString display) {
        ui->batteryLabel->setText(display);
    });
    updbus->DeviceChanged();

    if (updbus->hasBattery()) {
        ui->batteryFrame->setVisible(true);
    } else {
        ui->batteryFrame->setVisible(false);
    }

    PowerManager* pmanager = new PowerManager(this);

    UGlobalHotkeys* menuKey = new UGlobalHotkeys(this);
    menuKey->registerHotkey("Alt+F5");
    connect(menuKey, SIGNAL(activated(size_t)), this, SLOT(on_pushButton_clicked()));

    UGlobalHotkeys* infoKey = new UGlobalHotkeys(this);
    infoKey->registerHotkey("Alt+F6");
    connect(infoKey, SIGNAL(activated(size_t)), this, SLOT(pullDownGesture()));

    UGlobalHotkeys* runKey = new UGlobalHotkeys(this);
    runKey->registerHotkey("Alt+F2");
    connect(runKey, &UGlobalHotkeys::activated, [=]() {
        QMessageBox::warning(this, "Hi", "Hi", QMessageBox::Ok, QMessageBox::Ok);
    });

    infoPane = new InfoPaneDropdown(ndbus, updbus);
    infoPane->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(infoPane, SIGNAL(networkLabelChanged(QString)), this, SLOT(internetLabelChanged(QString)));
    connect(infoPane, SIGNAL(numNotificationsChanged(int)), this, SLOT(numNotificationsChanged(int)));
    connect(infoPane, SIGNAL(timerChanged(QString)), this, SLOT(setTimer(QString)));
    connect(infoPane, SIGNAL(timerVisibleChanged(bool)), this, SLOT(setTimerVisible(bool)));
    connect(infoPane, SIGNAL(timerEnabledChanged(bool)), this, SLOT(setTimerEnabled(bool)));
    infoPane->getNetworks();

    QSettings settings;

    QString loginSoundPath = settings.value("sounds/login", "").toString();
    if (loginSoundPath == "") {
        loginSoundPath = "/usr/share/sounds/contemporary/login.ogg";
        settings.setValue("sounds/login", loginSoundPath);
    }

    playSound(QUrl::fromLocalFile(loginSoundPath));

    ui->timer->setVisible(false);
    ui->timerIcon->setVisible(false);
    ui->timerIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(16, 16));
    ui->openingAppFrame->setVisible(false);

    if (QFile("/usr/bin/amixer").exists()) {
        ui->volumeSlider->setVisible(false);
    } else {
        ui->volumeFrame->setVisible(false);
    }

    ui->brightnessSlider->setVisible(false);
    ui->mprisFrame->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DBusNewService(QString name) {
    if (name.startsWith("org.mpris.MediaPlayer2.")) {
        if (!mprisDetectedApps.contains(name)) {
            mprisDetectedApps.append(name);
            if (mprisCurrentAppName == "") {
                mprisCurrentAppName = name;
            }
        }
    }

}

void MainWindow::pullDownGesture() {
    if (lockHide) {
        on_notifications_clicked();
    } else {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");

        anim->setStartValue(this->geometry());

        anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width() + 1, this->height()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        connect(anim, &QPropertyAnimation::finished, [=]() {
            hiding = false;
        });
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();

        lockHide = true;
        QTimer* timer = new QTimer();
        timer->setSingleShot(true);
        timer->setInterval(3000);
        connect(timer, &QTimer::timeout, [=]() {
            lockHide = false;
            timer->deleteLater();
        });
        timer->start();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->accept();
}

void MainWindow::on_pushButton_clicked()
{
    this->setFocus();
    Menu* m = new Menu(this);
    m->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //m->setGeometry(this->x(), this->y() + this->height() - 1, m->width(), screenGeometry.height() - (this->height() + (this->y() - screenGeometry.y())) + 1);
    m->setGeometry(this->x() - m->width(), this->y() + this->height() - 1, m->width(), screenGeometry.height() - (this->height() + (this->y() - screenGeometry.y())) + 1);
    m->show();
    m->setFocus();

    lockHide = true;
    connect(m, SIGNAL(appOpening(QString,QIcon)), this, SLOT(openingApp(QString,QIcon)));
    connect(m, &Menu::menuClosing, [=]() {
        lockHide = false;
    });
}

void MainWindow::reloadWindows() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QProcess p; //Get all open windows
    p.start("wmctrl -lpG");
    p.waitForStarted();
    while (p.state() != 0) {
        QApplication::processEvents(); //Don't block UI while reloading windows
    }

    QList<WmWindow*> *wlist = new QList<WmWindow*>();

    int hideTop = screenGeometry.y();

    int okCount = 0;
    QString output(p.readAllStandardOutput());
    for (QString window : output.split("\n")) {
        QStringList parts = window.split(" ");
        parts.removeAll("");
        if (parts.length() >= 9) {
            if (parts[2].toInt() != QCoreApplication::applicationPid()) {
                WmWindow *w = new WmWindow(this);
                w->setPID(parts[2].toInt());
                QString title;
                for (int i = 8; i != parts.length(); i++) {
                    title = title.append(" " + parts[i]);
                }
                title = title.remove(0, 1);
                if (title.length() > 47) {
                    title.truncate(47);
                    title.append("...");
                }

                if (parts[3].toInt() >= this->x() &&
                        parts[4].toInt() - 50 <= screenGeometry.y() + this->height() &&
                        parts[4].toInt() - 50 - this->height() < hideTop) {
                    hideTop = parts[4].toInt() - 50 - this->height();
                }

                w->setTitle(title);

                for (WmWindow *wi : *windowList) {
                    if (wi->title() == w->title()) {
                        okCount++;
                    }
                }

                wlist->append(w);
            }
        }
    }

    if (hideTop + this->height() <= screenGeometry.y()) {
        hideTop = screenGeometry.y() - this->height();
    }

    //int row = 0, column = 0;
    if (okCount != wlist->count() || wlist->count() < windowList->count()) {
        //FlowLayout* layout = new FlowLayout();
        //layout->setSpacing(6);

        windowList = wlist;

        QLayoutItem* item;
        while ((item = ui->windowList->layout()->takeAt(0)) != NULL) {
            ui->windowList->layout()->removeItem(item);
            delete item->widget();
            delete item;
        }
        for (WmWindow *w : *windowList) {
            QPushButton *button = new QPushButton();
            button->setText(w->title());
            QSignalMapper* mapper = new QSignalMapper(this);
            connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
            mapper->setMapping(button, w->title());
            connect(mapper, SIGNAL(mapped(QString)), this, SLOT(activateWindow(QString)));
            ui->windowList->layout()->addWidget(button);
            //layout->addWidget(button);

            /*column++;
            if (column == 4) {
                column = 0;
                row++;
            }*/
        }

        ui->centralWidget->adjustSize();
        ui->openingAppFrame->setVisible(false);
        ui->windowList->layout()->setGeometry(ui->windowList->layout()->geometry());
        this->setFixedSize(this->sizeHint());
        //delete ui->windowList->layout();

        //ui->windowList->setLayout(layout);
    }

    if (!lockHide) {
        if (hideTop != this->hideTop) {
            this->hideTop = hideTop;
            QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
            anim->setStartValue(this->geometry());
            anim->setEndValue(QRect(this->x(), hideTop, screenGeometry.width() + 1, this->height()));
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();

            //this->setGeometry(QRect(this->x(), -100, screenGeometry.width(), this->height()));

            if (hideTop == screenGeometry.y()) {
                hiding = false;
            } else {
                hiding = true;
            }

            ui->openingAppFrame->setVisible(false);
        }

        if (hideTop != screenGeometry.y()) {
            if (hiding) {
                if (QCursor::pos().y() <= this->y() + this->height() &&
                        QCursor::pos().x() > screenGeometry.x() &&
                        QCursor::pos().x() < screenGeometry.x() + screenGeometry.width()) {
                    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
                    anim->setStartValue(this->geometry());

                    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width() + 1, this->height()));
                    anim->setDuration(500);
                    anim->setEasingCurve(QEasingCurve::OutCubic);

                    connect(anim, &QPropertyAnimation::finished, [=]() {
                        hiding = false;
                    });
                    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                    anim->start();
                }
            } else {
                if (QCursor::pos().y() > screenGeometry.y() + this->height() ||
                        QCursor::pos().x() < screenGeometry.x() ||
                        QCursor::pos().x() > screenGeometry.x() + screenGeometry.width()) {
                    hiding = true;
                    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
                    anim->setStartValue(this->geometry());

                    anim->setEndValue(QRect(screenGeometry.x(), hideTop, screenGeometry.width() + 1, this->height()));
                    anim->setDuration(500);
                    anim->setEasingCurve(QEasingCurve::OutCubic);
                    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                    anim->start();
                }
            }
        }
    }

    ui->date->setText(QDateTime::currentDateTime().toString("ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

    mprisDetectedApps.clear();
    for (QString service : QDBusConnection::sessionBus().interface()->registeredServiceNames().value()) {
        DBusNewService(service);
    }

    if (mprisCurrentAppName != "") {
        ui->mprisFrame->setVisible(true);
        if (!mprisDetectedApps.contains(mprisCurrentAppName)) { //Service closed.
            if (mprisDetectedApps.count() > 0) { //Set to next app
                mprisCurrentAppName = mprisDetectedApps.first();
                ui->mprisFrame->setVisible(true);
            } else { //Set to no app. Make mpris controller invisible.
                mprisCurrentAppName = "";
                ui->mprisFrame->setVisible(false);
            }
        }
    } else { //Make mpris controller invisible
        ui->mprisFrame->setVisible(false);
    }

    if (ui->mprisFrame->isVisible()) {
        //Get Current Song Metadata
        QDBusMessage MetadataRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        MetadataRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "Metadata");

        QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(MetadataRequest));
        QVariantMap replyData;
        QDBusArgument arg(reply.value().variant().value<QDBusArgument>());

        arg >> replyData;

        QString title = "";
        QString artist = "";

        if (replyData.contains("xesam:title")) {
            title = replyData.value("xesam:title").toString();
        }

        if (replyData.contains("xesam:artist")) {
            QStringList artists = replyData.value("xesam:artist").toStringList();
            for (QString art : artists) {
                artist.append(art + ", ");
            }
            artist.remove(artist.length() - 2, 2);
        }

        if (title == "") {
            QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
            IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

            QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
            ui->mprisSongName->setText(reply.value().variant().toString());
        } else {
            if (artist == "") {
                ui->mprisSongName->setText(title);
            } else {
                ui->mprisSongName->setText(artist + " - " + title);
            }
        }

        //Get Playback Status
        QDBusMessage PlayStatRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        PlayStatRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "PlaybackStatus");
        QDBusReply<QVariant> PlayStat = QDBusConnection::sessionBus().call(PlayStatRequest);
        if (PlayStat.value().toString() == "Playing") {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
        } else {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));

        }

    }
}

void MainWindow::activateWindow(QString windowTitle) {
    QProcess::startDetached("wmctrl -a " + windowTitle);
}

void MainWindow::on_time_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::openingApp(QString AppName, QIcon AppIcon) {
    ui->appOpeningLabel->setText("Opening " + AppName);
    ui->appOpeningIcon->setPixmap(AppIcon.pixmap(16, 16));
    ui->openingAppFrame->setVisible(true);

    QTimer *timer = new QTimer(this);
    timer->setInterval(10000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [=]() {
        ui->openingAppFrame->setVisible(false);
    });
    timer->start();
}

void MainWindow::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QMainWindow::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void MainWindow::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void MainWindow::on_date_clicked()
{
    infoPane->show(InfoPaneDropdown::Clock);
}

void MainWindow::on_pushButton_2_clicked()
{
    //theWave *w = new theWave(infoPane);
    //w->show();

    this->setFocus();
    Menu* m = new Menu(this);
    m->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //m->setGeometry(this->x(), this->y() + this->height() - 1, m->width(), screenGeometry.height() - (this->height() + (this->y() - screenGeometry.y())) + 1);
    m->setGeometry(this->x() - m->width(), this->y() + this->height() - 1, m->width(), screenGeometry.height() - (this->height() + (this->y() - screenGeometry.y())) + 1);
    m->show(true);
    m->setFocus();

    lockHide = true;
    connect(m, SIGNAL(appOpening(QString,QIcon)), this, SLOT(openingApp(QString,QIcon)));
    connect(m, &Menu::menuClosing, [=]() {
        lockHide = false;
    });
}

void MainWindow::internetLabelChanged(QString display) {
    ui->networkLabel->setText(display);
}

void MainWindow::on_networkLabel_clicked()
{
    infoPane->show(InfoPaneDropdown::Network);
}

void MainWindow::on_notifications_clicked()
{
    infoPane->show(InfoPaneDropdown::Notifications);
}

void MainWindow::on_batteryLabel_clicked()
{
    infoPane->show(InfoPaneDropdown::Battery);
}

void MainWindow::on_volumeFrame_MouseEnter()
{
    ui->volumeSlider->setVisible(true);
    QPropertyAnimation* anim = new QPropertyAnimation(ui->volumeSlider, "geometry");
    anim->setStartValue(ui->volumeSlider->geometry());
    QRect endGeometry = ui->volumeSlider->geometry();
    endGeometry.setWidth(220);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();


    //Get Current Volume
    QProcess* mixer = new QProcess(this);
    mixer->start("amixer");
    mixer->waitForFinished();
    QString output(mixer->readAll());
    delete mixer;

    bool readLine = false;
    for (QString line : output.split("\n")) {
        if (line.startsWith(" ") && readLine) {
            if (line.startsWith("  Front Left:")) {
                if (line.contains("[off]")) {
                    ui->volumeSlider->setValue(0);
                } else {
                    QString percent = line.mid(line.indexOf("\[") + 1, 3).remove("\%").remove("\]");
                    ui->volumeSlider->setValue(percent.toInt());
                    ui->volumeSlider->setMaximum(100);
                }
            }
        } else {
            if (line.contains("'Master'")) {
                readLine = true;
            } else {
                readLine = false;
            }
        }
    }
}

void MainWindow::on_volumeFrame_MouseExit()
{
    QPropertyAnimation* anim = new QPropertyAnimation(ui->volumeSlider, "geometry");
    anim->setStartValue(ui->volumeSlider->geometry());
    QRect endGeometry = ui->volumeSlider->geometry();
    endGeometry.setWidth(0);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
    connect(anim, &QPropertyAnimation::finished, [=]() {
        ui->volumeSlider->setVisible(false);
    });
}

void MainWindow::on_volumeSlider_sliderMoved(int position)
{
    //Get Current Limits
    QProcess* mixer = new QProcess(this);
    mixer->start("amixer");
    mixer->waitForFinished();
    QString output(mixer->readAll());

    bool readLine = false;
    int limit;
    for (QString line : output.split("\n")) {
        if (line.startsWith(" ") && readLine) {
            if (line.startsWith("  Limits:")) {
                limit = line.split(" ").last().toInt();
            }
        } else {
            if (line.contains("'Master'")) {
                readLine = true;
            } else {
                readLine = false;
            }
        }
    }

    mixer->start("amixer set Master " + QString::number(limit * (position / (float) 100)) + " on");
    connect(mixer, SIGNAL(finished(int)), mixer, SLOT(deleteLater()));
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    on_volumeSlider_sliderMoved(value);

}

void MainWindow::on_brightnessFrame_MouseEnter()
{
    ui->brightnessSlider->setVisible(true);
    QPropertyAnimation* anim = new QPropertyAnimation(ui->brightnessSlider, "geometry");
    anim->setStartValue(ui->brightnessSlider->geometry());
    QRect endGeometry = ui->brightnessSlider->geometry();
    endGeometry.setWidth(220);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
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
    QPropertyAnimation* anim = new QPropertyAnimation(ui->brightnessSlider, "geometry");
    anim->setStartValue(ui->brightnessSlider->geometry());
    QRect endGeometry = ui->brightnessSlider->geometry();
    endGeometry.setWidth(0);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
    connect(anim, &QPropertyAnimation::finished, [=]() {
        ui->brightnessSlider->setVisible(false);
        anim->deleteLater();
    });

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
    QSoundEffect* volumeSound = new QSoundEffect();
    volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
    volumeSound->play();
    connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
    event->accept();
}

void MainWindow::numNotificationsChanged(int notifications) {
    QFont font = ui->notifications->font();
    if (notifications == 0) {
        font.setBold(false);
        ui->notifications->setText("No notifications");
    } else {
        font.setBold(true);
        if (notifications == 1) {
            ui->notifications->setText("1 notification");
        } else {
            ui->notifications->setText(QString::number(notifications) + " notifications");
        }
    }
    ui->notifications->setFont(font);
}

InfoPaneDropdown* MainWindow::getInfoPane() {
    return this->infoPane;
}

void MainWindow::on_pushButton_4_clicked()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    TouchKeyboard* keyboard = new TouchKeyboard();
    keyboard->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog | Qt::FramelessWindowHint);
    keyboard->setAttribute(Qt::WA_ShowWithoutActivating, true);
    keyboard->setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
    keyboard->show();
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
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "PlayPause"), QDBus::NoBlock);
}

void MainWindow::on_mprisBack_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Previous"), QDBus::NoBlock);
}

void MainWindow::on_pushButton_3_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Next"), QDBus::NoBlock);
}

void MainWindow::on_mprisSongName_clicked()
{
    QDBusConnection::sessionBus().call(QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2", "Raise"), QDBus::NoBlock);
}
