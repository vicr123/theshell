#include "endsessionwait.h"
#include "ui_endsessionwait.h"

extern void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);

EndSessionWait::EndSessionWait(shutdownType type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EndSessionWait)
{
    ui->setupUi(this);

    powerOffTimer = new QVariantAnimation();
    powerOffTimer->setStartValue(0);
    powerOffTimer->setEndValue(300);
    powerOffTimer->setDuration(30000);
    connect(powerOffTimer, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->idleProgressBar->setValue(value.toInt());
        ui->idleWarning->setText(tr("If you don't do anything, we'll power off for you in %1 seconds.").arg(QString::number(30 - (value.toInt() / 10))));
    });
    connect(powerOffTimer, &QVariantAnimation::finished, [=]() {
        //Power off the device
        this->type = powerOff;
        ui->label->setText(tr("Power Off"));

        //We need to use a QTimer to run the function on the event loop because we do something strange in this->showFullScreen()
        QTimer* invokeTimer = new QTimer();
        invokeTimer->setInterval(0);
        invokeTimer->setSingleShot(true);
        connect(invokeTimer, &QTimer::timeout, [=]() {
            invokeTimer->deleteLater();
            this->showFullScreen();
        });
        invokeTimer->start();
    });

    ui->slideOffFrame->installEventFilter(this);
    ui->ArrowUp->setPixmap(QIcon::fromTheme("go-up").pixmap(16, 16));

    if (!QApplication::arguments().contains("--debug")) {
        ui->DummyExit->setVisible(false);
    }

    switch (type) {
        case powerOff:
            ui->label->setText(tr("Power Off"));
            ui->askWhatToDo->setVisible(false);
            break;
        case reboot:
            ui->label->setText(tr("Reboot"));
            ui->askWhatToDo->setVisible(false);
            break;
        case logout:
            ui->label->setText(tr("Log out"));
            ui->askWhatToDo->setVisible(false);
            break;
        case dummy:
            ui->label->setText(tr("Dummy"));
            ui->askWhatToDo->setVisible(false);
            break;
        case ask:
            ui->poweringOff->setVisible(false);
    }

    this->type = type;
}

EndSessionWait::~EndSessionWait()
{
    delete ui;
}

void EndSessionWait::close() {
    if (this->type == slideOff) {
        tPropertyAnimation* anim = new tPropertyAnimation(ui->slideOffFrame, "geometry");
        anim->setStartValue(ui->slideOffFrame->geometry());
        anim->setEndValue(QRect(0, this->height(), this->width(), ui->slideOffFrame->height()));
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::InCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tPropertyAnimation::finished, [=] {
            this->reject();
        });
        anim->start();
    } else {
        this->reject();
    }
}

void EndSessionWait::showFullScreen() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (this->type == slideOff) {
        this->setAttribute(Qt::WA_TranslucentBackground);

        this->setGeometry(screenGeometry);
        QDialog::showFullScreen();
        QApplication::processEvents();

        this->layout()->removeWidget(ui->slideOffFrame);
        ui->slideOffFrame->setGeometry(0, this->height(), this->width(), ui->slideOffFrame->sizeHint().height());
        ui->slideOffFrame->setFixedHeight(ui->slideOffFrame->sizeHint().height());
        ui->slideOffFrame->setFixedWidth(this->width());

        tPropertyAnimation* anim = new tPropertyAnimation(ui->slideOffFrame, "geometry");
        anim->setStartValue(ui->slideOffFrame->geometry());
        anim->setEndValue(QRect(0, this->height() - ui->slideOffFrame->height(), this->width(), ui->slideOffFrame->height()));
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();

        ui->MainFrame->setVisible(false);
        this->setWindowOpacity(1.0);
    } else {
        ui->slideOffFrame->setVisible(false);
        QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
        anim->setDuration(250);
        anim->setStartValue(this->windowOpacity());
        if (this->type == ask) {
            anim->setEndValue(1.0);
        } else {
            anim->setEndValue(0.8);
        }

        if (!alreadyShowing) {
            alreadyShowing = true;
            this->setWindowOpacity(0.0);
            QDialog::showFullScreen();
            ui->terminateAppFrame->setGeometry(ui->terminateAppFrame->x(), ui->terminateAppFrame->y(), ui->terminateAppFrame->width(), 0);
            ui->terminateAppFrame->setVisible(false);
            ui->ExitFrameTop->resize(ui->ExitFrameTop->sizeHint());
            ui->ExitFrameBottom->resize(ui->ExitFrameBottom->sizeHint());
            anim->start(QAbstractAnimation::DeleteWhenStopped);
            powerOffTimer->start();
        } else {
            QParallelAnimationGroup* parallelAnimGroup = new QParallelAnimationGroup;
            QSequentialAnimationGroup* animGroup = new QSequentialAnimationGroup;

            {
                //Animate the "End Session" dialog out
                QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
                ui->askWhatToDo->setGraphicsEffect(fadeEffect);
                QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
                a->setDuration(250);
                a->setStartValue(1);
                a->setEndValue(0);
                animGroup->addAnimation(a);
            }

            {
                //Animate the "Ending Session" dialog in
                QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
                ui->poweringOff->setGraphicsEffect(fadeEffect);
                QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
                a->setDuration(250);
                a->setStartValue(0);
                a->setEndValue(1);
                animGroup->addAnimation(a);
            }

            connect(animGroup, &QSequentialAnimationGroup::currentAnimationChanged, [=](QAbstractAnimation* current) {
                if (animGroup->indexOfAnimation(current) == 1) {
                    ui->askWhatToDo->setVisible(false);
                    ui->poweringOff->setVisible(true);
                }
            });
            parallelAnimGroup->addAnimation(animGroup);
            parallelAnimGroup->addAnimation(anim);

            connect(parallelAnimGroup, SIGNAL(finished()), parallelAnimGroup, SLOT(deleteLater()));
            parallelAnimGroup->start();

            while (parallelAnimGroup->state() == QSequentialAnimationGroup::Running) {
                QApplication::processEvents();
            }
        }


        if (this->type != dummy && this->type != ask) {
            powerOffTimer->stop();
            powerOffTimer->setCurrentTime(0);
            /*QProcess p;
            p.start("wmctrl -lp");
            p.waitForStarted();
            while (p.state() != 0) {
                QApplication::processEvents();
            }

            QList<WmWindow*> *wlist = new QList<WmWindow*>();

            QString output(p.readAllStandardOutput());
            for (QString window : output.split("\n")) {
                QStringList parts = window.split(" ");
                parts.removeAll("");
                if (parts.length() >= 4) {
                    if (parts[2].toInt() != QCoreApplication::applicationPid()) {
                        WmWindow *w = new WmWindow();
                        w->setPID(parts[2].toInt());
                        QString title;
                        for (int i = 4; i != parts.length(); i++) {
                            title = title.append(" " + parts[i]);
                        }
                        title = title.remove(0, 1);

                        w->setTitle(title);
                        wlist->append(w);
                    }
                }
            }*/

            //Prepare a window list
            QList<WmWindow> wlist;

            //Get the current display
            Display* d = QX11Info::display();

            //Create list of all top windows and populate it
            QList<Window> TopWindows;

            Atom WindowListType;
            int format;
            unsigned long items, bytes;
            unsigned char *data;
            XGetWindowProperty(d, RootWindow(d, 0), XInternAtom(d, "_NET_CLIENT_LIST", true), 0L, (~0L),
                                            False, AnyPropertyType, &WindowListType, &format, &items, &bytes, &data);

            quint64 *windows = (quint64*) data;
            for (unsigned long i = 0; i < items; i++) {
                TopWindows.append((Window) windows[i]);

            }
            XFree(data);

            for (Window win : TopWindows) {
                XWindowAttributes attributes;

                int retval = XGetWindowAttributes(d, win, &attributes);
                unsigned long items, bytes;
                unsigned char *netWmName;
                XTextProperty wmName;
                int format;
                Atom ReturnType;
                retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_VISIBLE_NAME", False), 0, 1024, False,
                                   XInternAtom(d, "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, &netWmName);
                if (retval != 0 || netWmName == 0x0) {
                    retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_NAME", False), 0, 1024, False,
                                       AnyPropertyType, &ReturnType, &format, &items, &bytes, &netWmName);
                    if (retval != 0) {
                        retval = XGetWMName(d, win, &wmName);
                        if (retval == 1) {
                            retval = 0;
                        } else {
                            retval = 1;
                        }
                    }
                }
                if (retval == 0) {
                    WmWindow w;
                    w.setWID(win);

                    QString title;
                    if (netWmName) {
                        title = QString::fromLocal8Bit((char *) netWmName);
                        XFree(netWmName);
                    } else if (wmName.value) {
                        title = QString::fromLatin1((char *) wmName.value);
                        //XFree(wmName);
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

                    //Set the title of the window
                    w.setTitle(title);

                    //Make sure PID is not current application PID
                    if (w.PID() != QCoreApplication::applicationPid()) {
                        wlist.append(w);
                    }
                }
            }

            for (WmWindow window : wlist) {

                if (QApplication::arguments().contains("--debug")) {
                    if (!window.title().toLower().contains("theterminal") && !window.title().toLower().contains("qt creator")) {
                        sendMessageToRootWindow("_NET_CLOSE_WINDOW", window.WID());
                    }
                } else {
                    sendMessageToRootWindow("_NET_CLOSE_WINDOW", window.WID());
                }
                /*p.start("wmctrl -c " + window.title());
                p.waitForStarted();
                while (p.state() != 0) {
                    QApplication::processEvents();
                }*/
            }

            bool appsOpen = true;

            QProcess p;
            while (appsOpen) {
                appsOpen = false;
                p.start("wmctrl -lp");
                p.waitForStarted();
                while (p.state() != 0) {
                    QApplication::processEvents();
                }
                QString output(p.readAllStandardOutput());
                for (QString window : output.split("\n")) {
                    QStringList parts = window.split(" ");
                    parts.removeAll("");
                    if (parts.length() >= 4) {
                        if (parts[2].toInt() != QCoreApplication::applicationPid()) {
                            appsOpen = true;
                        }
                    }
                }
                QApplication::processEvents();
            }

            performEndSession();
        }
    }
}

void EndSessionWait::on_pushButton_clicked()
{
    QProcess p;
    p.start("wmctrl -lp");
    p.waitForStarted();
    while (p.state() != 0) {
        QApplication::processEvents();
    }

    QList<WmWindow> wlist;

    QString output(p.readAllStandardOutput());
    for (QString window : output.split("\n")) {
        QStringList parts = window.split(" ");
        parts.removeAll("");
        if (parts.length() >= 4) {
            if (parts[2].toInt() != QCoreApplication::applicationPid()) {
                WmWindow w;
                w.setPID(parts[2].toInt());
                QString title;
                for (int i = 4; i != parts.length(); i++) {
                    title = title.append(" " + parts[i]);
                }
                title = title.remove(0, 1);

                w.setTitle(title);
                wlist.append(w);
            }
        }
    }

    performEndSession();
}

void EndSessionWait::performEndSession() {
    QSettings settings;
    QString logoutSoundPath = settings.value("sounds/logout", "").toString();
    if (logoutSoundPath == "") {
        logoutSoundPath = "/usr/share/sounds/contemporary/logout.ogg";
        settings.setValue("sounds/logout", logoutSoundPath);
    }

    QMediaPlayer* sound = new QMediaPlayer();

    connect(sound, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(EndSessionNow()));
    connect(sound, &QMediaPlayer::stateChanged, [=]() {
        if (sound->state() == QMediaPlayer::StoppedState) {
            EndSessionNow();
        }
    });

    sound->setMedia(QUrl::fromLocalFile(logoutSoundPath));
    sound->play();

    QParallelAnimationGroup* animGroup = new QParallelAnimationGroup();
    QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
    ui->poweringOff->setGraphicsEffect(fadeEffect);
    QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
    a->setDuration(500);
    a->setStartValue(1);
    a->setEndValue(0);
    animGroup->addAnimation(a);

    QPropertyAnimation* opacity = new QPropertyAnimation(this, "windowOpacity");
    opacity->setDuration(250);
    opacity->setStartValue(this->windowOpacity());
    opacity->setEndValue(1.0);
    animGroup->addAnimation(opacity);

    animGroup->start();

    connect(a, &QPropertyAnimation::finished, [=]() {
        ui->poweringOff->setVisible(false);
    });
}

void EndSessionWait::EndSessionNow() {
    QDBusMessage message;
    QList<QVariant> arguments;
    arguments.append(true);
    switch (type) {
    case powerOff:
        //Power off the PC
        message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
        message.setArguments(arguments);
        QDBusConnection::systemBus().send(message);

        break;
    case reboot:
        //Reboot the PC
        message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Reboot");
        message.setArguments(arguments);
        QDBusConnection::systemBus().send(message);
        break;
    case logout:
        QApplication::exit(0);
    }
}

void EndSessionWait::on_pushButton_2_clicked()
{
    this->close();
}

void EndSessionWait::on_CancelAsk_clicked()
{
    this->close();
}

void EndSessionWait::on_PowerOff_clicked()
{
    this->type = powerOff;
    ui->label->setText(tr("Power Off"));
    this->showFullScreen();
}

void EndSessionWait::on_Reboot_clicked()
{
    this->type = reboot;
    ui->label->setText(tr("Reboot"));
    this->showFullScreen();
}

void EndSessionWait::on_LogOut_clicked()
{
    this->type = logout;
    ui->label->setText(tr("Log Out"));
    this->showFullScreen();
}

void EndSessionWait::on_Suspend_clicked()
{
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Suspend");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
    this->close();
}

void EndSessionWait::on_terminateApp_clicked()
{
    powerOffTimer->stop();
    powerOffTimer->setCurrentTime(0);
    QParallelAnimationGroup* group = new QParallelAnimationGroup();

    QVariantAnimation* topAnim = new QVariantAnimation();
    topAnim->setStartValue(ui->ExitFrameTop->height());
    topAnim->setEndValue(0);
    topAnim->setEasingCurve(QEasingCurve::OutCubic);
    topAnim->setDuration(250);
    connect(topAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->ExitFrameTop->setFixedHeight(value.toInt());
    });

    QVariantAnimation* bottomAnim = new QVariantAnimation();
    bottomAnim->setStartValue(ui->ExitFrameBottom->height());
    bottomAnim->setEndValue(0);
    bottomAnim->setEasingCurve(QEasingCurve::OutCubic);
    bottomAnim->setDuration(250);
    connect(bottomAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->ExitFrameBottom->setFixedHeight(value.toInt());
    });

    QVariantAnimation* midAnim = new QVariantAnimation();
    midAnim->setStartValue(ui->terminateAppFrame->height());
    midAnim->setEndValue(ui->terminateAppFrame->sizeHint().height());
    midAnim->setEasingCurve(QEasingCurve::OutCubic);
    midAnim->setDuration(250);
    connect(midAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->terminateAppFrame->setFixedHeight(value.toInt());
    });

    group->addAnimation(midAnim);
    group->addAnimation(topAnim);
    group->addAnimation(bottomAnim);

    group->start();

    connect(group, &QParallelAnimationGroup::finished, [=]() {
        ui->terminateAppFrame->setFixedHeight(ui->terminateAppFrame->sizeHint().height());
        ui->ExitFrameTop->setFixedHeight(0);
        ui->ExitFrameBottom->setFixedHeight(0);
    });
    ui->terminateAppFrame->setVisible(true);

    this->reloadAppList();
}

void EndSessionWait::reloadAppList() {
    QList<WmWindow> wlist;

    Display* d = QX11Info::display();
    QList<Window> TopWindows;

    Atom WindowListType;
    int format;
    unsigned long items, bytes;
    unsigned char *data;
    XGetWindowProperty(d, RootWindow(d, 0), XInternAtom(d, "_NET_CLIENT_LIST", true), 0L, (~0L),
                                    False, AnyPropertyType, &WindowListType, &format, &items, &bytes, &data);

    quint64 *windows = (quint64*) data;
    for (unsigned long i = 0; i < items; i++) {
        TopWindows.append((Window) windows[i]);

    }
    XFree(data);

    //XQueryTree(QX11Info::display(), RootWindow(d, 0), new Window(), new Window(), &ChildList, &NumOfChildren);
    for (Window win : TopWindows) {
        XWindowAttributes attributes;

        int retval = XGetWindowAttributes(d, win, &attributes);
        unsigned long items, bytes;
        unsigned char *netWmName;
        XTextProperty wmName;
        int format;
        Atom ReturnType;
        retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_VISIBLE_NAME", False), 0, 1024, False,
                           XInternAtom(d, "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, &netWmName);
        if (retval != 0 || netWmName == 0x0) {
            retval = XGetWindowProperty(d, win, XInternAtom(d, "_NET_WM_NAME", False), 0, 1024, False,
                               AnyPropertyType, &ReturnType, &format, &items, &bytes, &netWmName);
            if (retval != 0) {
                retval = XGetWMName(d, win, &wmName);
                if (retval == 1) {
                    retval = 0;
                } else {
                    retval = 1;
                }
            }
        }
        if (retval == 0) {
            WmWindow w;
            w.setWID(win);

            QString title;
            if (netWmName) {
                title = QString::fromLocal8Bit((char *) netWmName);
                XFree(netWmName);
            } else if (wmName.value) {
                title = QString::fromLatin1((char *) wmName.value);
                //XFree(wmName);
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

            if (w.PID() != QCoreApplication::applicationPid()) {
                wlist.append(w);
            }

        }
    }

    ui->listWidget->clear();
    for (WmWindow wi : wlist) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(wi.title() + " (PID " + QString::number(wi.PID()) + ")");
        item->setData(Qt::UserRole, QVariant::fromValue(wi.PID()));
        item->setIcon(wi.icon());
        ui->listWidget->addItem(item);
    }
}

void EndSessionWait::on_exitTerminate_clicked()
{
    powerOffTimer->start();
    QParallelAnimationGroup* group = new QParallelAnimationGroup();

    QVariantAnimation* topAnim = new QVariantAnimation();
    topAnim->setStartValue(ui->ExitFrameTop->height());
    topAnim->setEndValue(ui->ExitFrameTop->sizeHint().height());
    topAnim->setEasingCurve(QEasingCurve::OutCubic);
    topAnim->setDuration(500);
    topAnim->setKeyValueAt(0.5, 0);
    connect(topAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->ExitFrameTop->setFixedHeight(value.toInt());
    });

    QVariantAnimation* bottomAnim = new QVariantAnimation();
    bottomAnim->setStartValue(ui->ExitFrameBottom->height());
    bottomAnim->setEndValue(ui->ExitFrameBottom->sizeHint().height());
    bottomAnim->setEasingCurve(QEasingCurve::OutCubic);
    bottomAnim->setDuration(500);
    bottomAnim->setKeyValueAt(0.5, 0);
    connect(bottomAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->ExitFrameBottom->setFixedHeight(value.toInt());
    });

    QVariantAnimation* midAnim = new QVariantAnimation();
    midAnim->setStartValue(ui->terminateAppFrame->height());
    midAnim->setEndValue(0);
    midAnim->setEasingCurve(QEasingCurve::OutCubic);
    midAnim->setDuration(500);
    connect(midAnim, &QVariantAnimation::valueChanged, [=](QVariant value) {
        ui->terminateAppFrame->setFixedHeight(value.toInt());
    });

    group->addAnimation(midAnim);
    group->addAnimation(topAnim);
    group->addAnimation(bottomAnim);

    group->start();

    connect(group, &QParallelAnimationGroup::finished, [=]() {
        ui->terminateAppFrame->setFixedHeight(0);
        ui->ExitFrameTop->setFixedHeight(ui->ExitFrameTop->sizeHint().height());
        ui->ExitFrameBottom->setFixedHeight(ui->ExitFrameBottom->sizeHint().height());
        ui->terminateAppFrame->setVisible(false);
    });
}

void EndSessionWait::on_pushButton_5_clicked()
{
    //Send SIGTERM to app
    kill(ui->listWidget->selectedItems().first()->data(Qt::UserRole).value<unsigned long>(), SIGTERM);
    QThread::sleep(1);
    reloadAppList();
}

void EndSessionWait::on_pushButton_4_clicked()
{
    //Send SIGKILL to app
    kill(ui->listWidget->selectedItems().first()->data(Qt::UserRole).value<unsigned long>(), SIGKILL);
    QThread::sleep(1);
    reloadAppList();
}

void EndSessionWait::on_listWidget_currentRowChanged(int currentRow)
{
    if (currentRow == -1) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
    } else {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_4->setEnabled(true);
    }
}

void EndSessionWait::on_DummyExit_clicked()
{
    //Fake Exit
    this->type = dummy;
    ui->label->setText(tr("Dummy"));
    this->showFullScreen();
}

void EndSessionWait::reject() {
    powerOffTimer->stop();
    powerOffTimer->setCurrentTime(0);

    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(250);
    anim->setStartValue(this->windowOpacity());
    anim->setEndValue(0.0);
    connect(anim, &QPropertyAnimation::finished, [=]() {
        QDialog::reject();
        anim->deleteLater();
    });
    anim->start();
}

void EndSessionWait::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    if (this->type == slideOff) {
        int alpha;
        alpha = ((float) ui->slideOffFrame->y() / (float) (this->height() - ui->slideOffFrame->height())) * 105;
        painter.setBrush(QColor(0, 0, 0, 105 - alpha + 150));
        painter.setPen(QColor(0, 0, 0, 0));
        painter.drawRect(0, 0, this->width(), ui->slideOffFrame->y());
        painter.setBrush(this->palette().brush(QPalette::Window));
        painter.drawRect(0, ui->slideOffFrame->y(), this->width(), ui->slideOffFrame->height() + this->height());
    } else {
        painter.setBrush(this->palette().brush(QPalette::Window));
        painter.setPen(Qt::transparent);
        painter.drawRect(event->rect());
    }
}

void EndSessionWait::mousePressEvent(QMouseEvent *event) {
    Q_UNUSED(event)

    if (this->type == slideOff) {
        this->close();
    }
}

bool EndSessionWait::eventFilter(QObject *obj, QEvent *eve) {
    if (obj == ui->slideOffFrame) {
        if (eve->type() == QEvent::MouseButtonPress) {
            QMouseEvent* event = (QMouseEvent*) eve;
            pressLocation = event->y();
            return true;
        } else if (eve->type() == QEvent::MouseMove) {
            int top = QCursor::pos().y() - this->y() - pressLocation;
            //if (top < 0) top = 0;
            ui->slideOffFrame->move(0, top);
            this->update();
            return true;
        } else if (eve->type() == QEvent::MouseButtonRelease) {
            if (ui->slideOffFrame->y() < 50) {

                tPropertyAnimation* anim = new tPropertyAnimation(ui->slideOffFrame, "geometry");
                anim->setStartValue(ui->slideOffFrame->geometry());
                anim->setEndValue(QRect(0, -ui->slideOffFrame->height(), this->width(), ui->slideOffFrame->height()));
                anim->setDuration(250);
                anim->setEasingCurve(QEasingCurve::InCubic);
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                connect(anim, SIGNAL(finished()), this, SLOT(update()));
                connect(anim, &tPropertyAnimation::finished, [=] {
                    this->type = powerOff;

                    this->setAttribute(Qt::WA_TranslucentBackground, false);
                    this->update();
                    ui->label->setText(tr("Power Off"));
                    ui->MainFrame->setVisible(true);
                    ui->askWhatToDo->setVisible(false);
                    this->showFullScreen();
                });
                anim->start();

            } else if (ui->slideOffFrame->y() > this->height() - ui->slideOffFrame->height()) {
                this->close();
            } else {
                tPropertyAnimation* anim = new tPropertyAnimation(ui->slideOffFrame, "geometry");
                anim->setStartValue(ui->slideOffFrame->geometry());
                anim->setEndValue(QRect(0, this->height() - ui->slideOffFrame->height(), this->width(), ui->slideOffFrame->height()));
                anim->setDuration(500);
                anim->setEasingCurve(QEasingCurve::OutBounce);
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                connect(anim, SIGNAL(valueChanged(QVariant)), this, SLOT(update()));
                anim->start();
            }
            return true;
        } else if (eve->type() == QEvent::Paint) {
            QPaintEvent* event = (QPaintEvent*) eve;
            QPainter painter(ui->slideOffFrame);
            painter.setBrush(ui->slideOffFrame->palette().brush(QPalette::Window));
            painter.setPen(Qt::transparent);
            painter.drawRect(event->rect());
            painter.setPen(ui->slideOffFrame->palette().color(QPalette::WindowText));
            painter.drawLine(0, 0, ui->slideOffFrame->width(), 0);
        }
    }
    return false;
}
