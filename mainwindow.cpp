#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    windowList = new QList<WmWindow*>();

    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(reloadWindows()));
    timer->start();

    NotificationDBus* ndbus = new NotificationDBus(this);

    UPowerDBus* updbus = new UPowerDBus(ndbus, this);
    connect(updbus, &UPowerDBus::updateDisplay, [=](QString display) {
        ui->batteryLabel->setText(display);
    });
    updbus->DeviceChanged();

    grabKeys();

    //XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_L), )

    //this->grabKeyboard();

    /*UGlobalHotkeys* hotkeyManager = new UGlobalHotkeys(this);
    hotkeyManager->registerHotkey("Alt+F1");
    connect(hotkeyManager, SIGNAL(activated(size_t)), this, SLOT(on_pushButton_clicked()));*/
}

MainWindow::~MainWindow()
{
    KeyCode LSuper = XKeysymToKeycode(QX11Info::display(), XK_Meta_L);
    XUngrabKey(QX11Info::display(), LSuper, AnyModifier, RootWindow(QX11Info::display(), 0));
    KeyCode RSuper = XKeysymToKeycode(QX11Info::display(), XK_Meta_R);
    XUngrabKey(QX11Info::display(), RSuper, AnyModifier, RootWindow(QX11Info::display(), 0));
    XUngrabKey(QX11Info::display(), 124, AnyModifier, RootWindow(QX11Info::display(), 0));


    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    Menu* m = new Menu(this);
    m->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    m->setGeometry(this->x(), this->y() + this->height(), m->width(), m->height());
    m->show();
    m->setFocus();
}

void MainWindow::reloadWindows() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QProcess p;
    p.start("wmctrl -lpG");
    p.waitForStarted();
    while (p.state() != 0) {
        QApplication::processEvents();
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

    int row = 0, column = 0;
    if (okCount != wlist->count() || wlist->count() < windowList->count()) {
        windowList = wlist;

        QLayoutItem* item;
        while ((item = ui->horizontalLayout_2->takeAt(0)) != NULL) {
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
            ui->horizontalLayout_2->addWidget(button, row, column);

            column++;
            if (column == 4) {
                column = 0;
                row++;
            }
        }
    }


    if (hideTop != this->hideTop) {
        this->hideTop = hideTop;
        QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
        anim->setStartValue(this->geometry());
        anim->setEndValue(QRect(this->x(), hideTop, screenGeometry.width() + 1, this->height()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();

        //this->setGeometry(QRect(this->x(), -100, screenGeometry.width(), this->height()));

        if (hideTop == screenGeometry.y()) {
            hiding = false;
        } else {
            hiding = true;
        }

    }

    if (hideTop != screenGeometry.y()) {
        int ypos = QCursor::pos().y();
        int xpos = QCursor::pos().x();
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
                anim->start();
            }
        }
    }

    ui->date->setText(QDateTime::currentDateTime().toString("ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
}

void MainWindow::activateWindow(QString windowTitle) {
    QProcess::startDetached("wmctrl -a " + windowTitle);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Meta:
        QMessageBox::warning(this, "Whoa", "Whoa");
    case Qt::Key_PowerOff:
        QMessageBox::warning(this, "Off", "Off");

    default:
        event->ignore();
    }

    grabKeys();
}

void MainWindow::on_time_clicked()
{
    //InfoPaneDropdown *dropdown = new InfoPaneDropdown(this);
    //dropdown->show(InfoPaneDropdown::Clock);
}

void MainWindow::grabKeys() {
    bool success;
    KeyCode LSuper = XKeysymToKeycode(QX11Info::display(), XK_Meta_L);
    success = XGrabKey(QX11Info::display(), LSuper, AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    KeyCode RSuper = XKeysymToKeycode(QX11Info::display(), XK_Meta_R);
    success = XGrabKey(QX11Info::display(), RSuper, AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    success = XGrabKey(QX11Info::display(), 124, AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    success = XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F2), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);

    //XGrabKeyboard(QX11Info::display(), RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync, CurrentTime);
}
