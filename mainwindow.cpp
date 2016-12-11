#include "mainwindow.h"
#include "ui_mainwindow.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);
extern DbusEvents* DBusEvents;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Set the menu of the MPRIS Media Player selection to a new menu.
    //Items will be populated during the update event.
    QMenu* mprisSelectionMenu = new QMenu();
    ui->mprisSelection->setMenu(mprisSelectionMenu);
    connect(mprisSelectionMenu, &QMenu::aboutToShow, [=]() {
        pauseMprisMenuUpdate = true;
    });
    connect(mprisSelectionMenu, &QMenu::aboutToHide, [=]() {
        pauseMprisMenuUpdate = false;
    });

    //Connect signals related to multiple monitor management
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
    connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));

    //Create the gateway and set required flags
    gatewayMenu = new Menu(this);
    gatewayMenu->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(gatewayMenu, &Menu::menuClosing, [=]() {
        lockHide = false;
    });

    this->setAttribute(Qt::WA_AlwaysShowToolTips, true);

    FlowLayout* flow = new FlowLayout(ui->windowList);
    ui->windowList->setLayout(flow);

    //Create the update event timer and start it
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(doUpdate()));
    timer->start();

    NotificationDBus* ndbus = new NotificationDBus(this);

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

    DBusEvents = new DbusEvents(ndbus);

    infoPane = new InfoPaneDropdown(ndbus, updbus);
    infoPane->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    connect(infoPane, SIGNAL(networkLabelChanged(QString)), this, SLOT(internetLabelChanged(QString)));
    connect(infoPane, SIGNAL(numNotificationsChanged(int)), this, SLOT(numNotificationsChanged(int)));
    connect(infoPane, SIGNAL(timerChanged(QString)), this, SLOT(setTimer(QString)));
    connect(infoPane, SIGNAL(timerVisibleChanged(bool)), this, SLOT(setTimerVisible(bool)));
    connect(infoPane, SIGNAL(timerEnabledChanged(bool)), this, SLOT(setTimerEnabled(bool)));
    connect(infoPane, &InfoPaneDropdown::notificationsSilencedChanged, [=](bool silenced) {
        ui->notifications->setShowDisabled(silenced);
    });
    infoPane->getNetworks();

    QString loginSoundPath = settings.value("sounds/login", "").toString();
    if (loginSoundPath == "") {
        loginSoundPath = "/usr/share/sounds/contemporary/login.ogg";
        settings.setValue("sounds/login", loginSoundPath);
    }

    playSound(QUrl::fromLocalFile(loginSoundPath));

    ui->timer->setVisible(false);
    ui->timerIcon->setVisible(false);
    ui->timerIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(16, 16));

    if (QFile("/usr/bin/amixer").exists()) {
        ui->volumeSlider->setVisible(false);
    } else {
        ui->volumeFrame->setVisible(false);
    }

    ui->brightnessSlider->setVisible(false);
    ui->mprisFrame->setVisible(false);

    this->setFocusPolicy(Qt::NoFocus);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::DBusNewService(QString name) {
    if (name.startsWith("org.mpris.MediaPlayer2.")) {
        if (!mprisDetectedApps.contains(name)) {
            QDBusConnection::sessionBus().connect(name, "/org/mpris/MediaPlayer2/", "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(updateMpris()));
            mprisDetectedApps.append(name);
            if (mprisCurrentAppName == "") {
                mprisCurrentAppName = name;
            }
            updateMpris();
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

void MainWindow::on_openMenu_clicked()
{
    openMenu();
}

void MainWindow::doUpdate() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

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
                ui->desktopName->setText("Desktop " + QString::number(currentDesktop + 1));
            } else {
                ui->desktopName->setText(QString(nameList.at(currentDesktop)));
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

                if (!noIcon) {
                    retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_ICON", False), 2, width * height * 4, False,
                                       XA_CARDINAL, &icReturnType, &icFormat, &icItems, &icBytes, &icon);

                    QImage image(width, height, QImage::Format_ARGB32);

                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width * 8; x = x + 8) {
                            unsigned long a, r, g, b;

                            b = (icon[y * width * 8 + x + 0]);
                            g = (icon[y * width * 8 + x + 1]);
                            r = (icon[y * width * 8 + x + 2]);
                            a = (icon[y * width * 8 + x + 3]);

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
            if (w.PID() == QApplication::applicationPid()) {
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

                    delete[] atoms;
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

                menu->addSection(w.icon(), "For " + w.title());
                menu->addAction(QIcon::fromTheme("window-close"), "Close", [=]() {
                    sendMessageToRootWindow("_NET_CLOSE_WINDOW", w.WID());
                });

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
    oldActiveWindow = active;

    if (!lockHide) { //Check for move lock
        if (hideTop < screenGeometry.y()) {
            if (attentionDemandingWindows > 0) {
                hideTop = screenGeometry.y() - this->height() + 2;
            }
        }
        if (hideTop != this->hideTop || forceWindowMove) { //Check if we need to move out of the way
            this->hideTop = hideTop;
            QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
            anim->setStartValue(this->geometry());
            anim->setEndValue(QRect(screenGeometry.x(), hideTop, screenGeometry.width() + 1, this->height()));
            anim->setDuration(500);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, &QPropertyAnimation::finished, [=]() {
                int adjustLeft = 0;
                adjustLeft = adjustLeft + ui->openMenu->width();
                if (ui->desktopsFrame->isVisible()) {
                    adjustLeft = adjustLeft + ui->openMenu->width();
                }
                //ui->windowList->layout()->setGeometry(ui->horizontalLayout_4->geometry().adjusted(adjustLeft, 0, 0, 0));
            });
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            anim->start();

            if (hideTop == screenGeometry.y()) {
                hiding = false;
            } else {
                hiding = true;
            }
        }

        if (hideTop != screenGeometry.y()) {
            if (hiding) {
                if (QCursor::pos().y() <= this->y() + this->height() &&
                        QCursor::pos().x() > screenGeometry.x() &&
                        QCursor::pos().x() < screenGeometry.x() + screenGeometry.width()) {
                    //Move away from the whole screen.
                    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
                    anim->setStartValue(this->geometry());

                    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width() + 1, this->height()));
                    anim->setDuration(500);
                    anim->setEasingCurve(QEasingCurve::OutCubic);

                    connect(anim, &QPropertyAnimation::finished, [=]() {
                        int adjustLeft = 0;
                        adjustLeft = adjustLeft + ui->openMenu->width();
                        if (ui->desktopsFrame->isVisible()) {
                            adjustLeft = adjustLeft + ui->openMenu->width();
                        }
                        //ui->windowList->layout()->setGeometry(ui->horizontalLayout_4->geometry().adjusted(adjustLeft, 0, 0, 0));
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
    forceWindowMove = false;

    //Update date and time
    ui->date->setText(QDateTime::currentDateTime().toString("ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().time().toString(Qt::TextDate));

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
}

void MainWindow::updateMpris() {
    if (ui->mprisFrame->isVisible()) {
        if (!pauseMprisMenuUpdate) {
            if (mprisDetectedApps.count() > 1) {
                QMenu* menu = ui->mprisSelection->menu();
                menu->clear();
                for (QString app : mprisDetectedApps) {
                    QAction* action = new QAction(NULL);
                    action->setData(app);
                    action->setCheckable(true);
                    if (mprisCurrentAppName == app) {
                        action->setChecked(true);
                    }
                    action->setText(app.remove("org.mpris.MediaPlayer2."));
                    menu->addAction(action);
                }
                //ui->mprisSelection->setMenu(menu);
                ui->mprisSelection->setVisible(true);
            } else {
                ui->mprisSelection->setVisible(false);
            }
        }

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

        if (title == "") {
            QDBusMessage IdentityRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
            IdentityRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2" << "Identity");

            QDBusReply<QDBusVariant> reply(QDBusConnection::sessionBus().call(IdentityRequest));
            ui->mprisSongName->setText(reply.value().variant().toString());
        } else {
            if (artist == "") {
                ui->mprisSongName->setText(title);
            } else {
                ui->mprisSongName->setText(artist + " · " + title);
            }
        }

        //Get Playback Status
        QDBusMessage PlayStatRequest = QDBusMessage::createMethodCall(mprisCurrentAppName, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
        PlayStatRequest.setArguments(QList<QVariant>() << "org.mpris.MediaPlayer2.Player" << "PlaybackStatus");
        QDBusReply<QVariant> PlayStat = QDBusConnection::sessionBus().call(PlayStatRequest);
        if (PlayStat.value().toString() == "Playing") {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-pause"));
            mprisPlaying = true;
        } else {
            ui->mprisPause->setIcon(QIcon::fromTheme("media-playback-start"));
            mprisPlaying = false;
        }

    }
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
    /*this->setFocus();
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
    });*/

    openMenu(true);
}

void MainWindow::internetLabelChanged(QString display) {
    if (display == "Flight Mode") {
        ui->networkLabel->setPixmap(getIconFromTheme("flight.svg", this->palette().color(QPalette::Window)).pixmap(16, 16));
    } else {
        ui->networkLabel->setText(display);
    }
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

    QVariantAnimation* anim = new QVariantAnimation;
    connect(anim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->volumeSlider->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->setStartValue(ui->volumeSlider->width());
    anim->setEndValue(150);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();

    /*
    QPropertyAnimation* anim = new QPropertyAnimation(ui->volumeSlider, "geometry");
    anim->setStartValue(ui->volumeSlider->geometry());
    QRect endGeometry = ui->volumeSlider->geometry();
    endGeometry.setWidth(220);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();*/


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
                    QString percent = line.mid(line.indexOf("\[") + 1, 3).remove("\%").remove("]");
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

    QVariantAnimation* anim = new QVariantAnimation;
    connect(anim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->volumeSlider->setFixedWidth(value.toInt());
    });
    connect(anim, &QVariantAnimation::finished, [=]() {
        ui->volumeSlider->setVisible(false);
        anim->deleteLater();
    });
    anim->setStartValue(ui->volumeSlider->width());
    anim->setEndValue(0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start();
    /*
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
    });*/
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
    QVariantAnimation* anim = new QVariantAnimation;
    connect(anim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->brightnessSlider->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->setStartValue(ui->brightnessSlider->width());
    anim->setEndValue(150);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    /*QPropertyAnimation* anim = new QPropertyAnimation(ui->brightnessSlider, "geometry");
    anim->setStartValue(ui->brightnessSlider->geometry());
    QRect endGeometry = ui->brightnessSlider->geometry();
    endGeometry.setWidth(220);
    anim->setEndValue(endGeometry);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();*/

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

    QVariantAnimation* anim = new QVariantAnimation;
    connect(anim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->brightnessSlider->setFixedWidth(value.toInt());
    });
    connect(anim, &QVariantAnimation::finished, [=]() {
        ui->brightnessSlider->setVisible(false);
        anim->deleteLater();
    });
    anim->setStartValue(ui->brightnessSlider->width());
    anim->setEndValue(0);
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->start();

    /*
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
    });*/

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
    if (this->attentionDemandingWindows > 0) {
        if (!warningAnimCreated) {
            warningAnimCreated = true;
            QVariantAnimation* anim = new QVariantAnimation(this);
            anim->setStartValue(0);
            anim->setEndValue(this->width());
            anim->setEasingCurve(QEasingCurve::OutBounce);
            anim->setDuration(1000);
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim, &QVariantAnimation::valueChanged, [=](QVariant var) {
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
        painter.setPen(this->palette().color(QPalette::WindowText));
        painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
        warningAnimCreated = false;
    }
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

void MainWindow::openMenu(bool openTotheWave, bool startListening) {
    if (gatewayMenu->isVisible()) {
        if (openTotheWave) {
            gatewayMenu->show(openTotheWave, startListening);

        }
    } else {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        gatewayMenu->setGeometry(this->x() - gatewayMenu->width(), this->y() + this->height() - 1, gatewayMenu->width(), screenGeometry.height() - (this->height() + (this->y() - screenGeometry.y())) + 1);
        gatewayMenu->show(openTotheWave, startListening);
        gatewayMenu->setFocus();

        lockHide = true;
    }
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
    mprisCurrentAppName = arg1->data().toString();
}
