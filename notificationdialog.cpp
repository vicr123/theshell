#include "notificationdialog.h"
#include "ui_notificationdialog.h"

extern QIcon getIconFromTheme(QString name, QColor textColor);

NotificationDialog::NotificationDialog(QString appName, QString title, QString body, QStringList actions, int id, QVariantMap hints, int timeout, notificationType type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotificationDialog)
{
    ui->setupUi(this);

    this->hints = hints;
    this->setAttribute(Qt::WA_X11NetWmWindowTypeNotification, true);

    /*QBrush background = this->palette().background();
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, pal.foreground());
    pal.setColor(QPalette::Button, pal.foreground().color());
    pal.setColor(QPalette::ButtonText, background.color());
    pal.setBrush(QPalette::Foreground, background);
    //pal().background().setColor(this->palette().foreground().color());
    //pal().foreground().setColor(background);
    this->setPalette(pal);*/

    ui->appName->setText(appName);
    switch (type) {
    case normalType:
        ui->notificationType->setCurrentIndex(0);
        ui->title->setText(title);
        ui->body->setText(body);
        ui->page2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
        break;
    case callType:
        ui->notificationType->setCurrentIndex(1);
        ui->title1->setText(title);
        ui->body1->setText(body);
        break;
    }

    this->setFixedHeight(this->sizeHint().height());

    for (int i = 0; i < actions.length(); i = i + 2) {
        QString action = actions.at(i);
        QString readable = actions.at(i + 1);

        QPushButton *button = new QPushButton();
        button->setText(readable);
        //button->setPalette(pal);
        connect(button, &QPushButton::clicked, [=]() {
            uint id2 = id;
            QString key = action;
            dbusParent->invokeAction(id2, key);
        });
        ui->actionsLayout->addWidget(button);
    }

    this->setAttribute(Qt::WA_ShowWithoutActivating, true);

    this->id = id;
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //w.setGeometry(screenGeometry.x() - 1, screenGeometry.y(), screenGeometry.width(), w.height());
    this->setGeometry(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));

    QColor color = this->palette().color(QPalette::Window);

    QIcon appIcon;
    bool foundAppIcon = false;
    QString filename = hints.value("desktop-entry", "").toString() + ".desktop";

    QDir appFolder("/usr/share/applications/");
    QDirIterator* iterator = new QDirIterator(appFolder, QDirIterator::Subdirectories);

    while (iterator->hasNext()) {
        iterator->next();
        QFileInfo info = iterator->fileInfo();
        if (info.fileName() == filename || info.baseName().toLower() == appName.toLower()) {
            QFile file(info.filePath());
            file.open(QFile::ReadOnly);
            QString appinfo(file.readAll());

            QStringList desktopLines;
            QString currentDesktopLine;
            for (QString desktopLine : appinfo.split("\n")) {
                if (desktopLine.startsWith("[") && currentDesktopLine != "") {
                    desktopLines.append(currentDesktopLine);
                    currentDesktopLine = "";
                }
                currentDesktopLine.append(desktopLine + "\n");
            }
            desktopLines.append(currentDesktopLine);

            for (QString desktopPart : desktopLines) {
                for (QString line : desktopPart.split("\n")) {
                    if (line.startsWith("icon=", Qt::CaseInsensitive)) {
                        QString iconname = line.split("=")[1];
                        if (QFile(iconname).exists()) {
                            appIcon = QIcon(iconname);
                        } else {
                            appIcon = QIcon::fromTheme(iconname, QIcon::fromTheme("application-x-executable"));
                        }
                        foundAppIcon = true;
                    }
                }
            }
        }
    }

    delete iterator;

    if (foundAppIcon) {
        ui->appIcon->setPixmap(appIcon.pixmap(16, 16));
    } else {
        ui->appIcon->setVisible(false);
    }

    // Don't forget to add extra categories to the notificationdbus class too!

    QSize iconSize(24, 24);
    if (hints.keys().contains("category")) {
        QString category = hints.value("category").toString();
        if (category == "network.connected") {
            ui->label->setPixmap(QIcon::fromTheme("network-connect").pixmap(iconSize));
        } else if (category == "network.disconnected") {
            ui->label->setPixmap(QIcon::fromTheme("network-disconnect").pixmap(iconSize));
        } else if (category == "email.arrived") {
            ui->label->setPixmap(QIcon::fromTheme("mail-receive").pixmap(iconSize));
        } else if (category == "battery.charging") {
            ui->label->setPixmap(getIconFromTheme("battery-charging.svg", color).pixmap(iconSize));
        } else if (category == "battery.charged") {
            ui->label->setPixmap(getIconFromTheme("battery-charged.svg", color).pixmap(iconSize));
        } else if (category == "battery.discharging") {
            ui->label->setPixmap(getIconFromTheme("battery-not-charging.svg", color).pixmap(iconSize));
        } else if (category == "battery.low") {
            ui->label->setPixmap(getIconFromTheme("battery-low.svg", color).pixmap(iconSize));
        } else if (category == "battery.critical") {
            ui->label->setPixmap(getIconFromTheme("battery-critical.svg", color).pixmap(iconSize));
        } else if (category == "device.added") {
            ui->label->setPixmap(getIconFromTheme("connect.svg", color).pixmap(iconSize));
        } else if (category == "device.removed") {
            ui->label->setPixmap(getIconFromTheme("disconnect.svg", color).pixmap(iconSize));
        } else if (category == "call.incoming") {
            ui->label->setPixmap(QIcon::fromTheme("call-start").pixmap(iconSize));
        } else {
            ui->label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(iconSize));
        }
    } else {
        if (hints.keys().contains("urgency")) {
            QChar urgency = hints.value("urgency").toChar();
            if (urgency == 0) {
                ui->label->setPixmap(QIcon::fromTheme("dialog-information").pixmap(iconSize));
            } else if (urgency == 1) {
                ui->label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(iconSize));
            } else if (urgency == 2) {
                ui->label->setPixmap(QIcon::fromTheme("dialog-error").pixmap(iconSize));
            }
        } else {
            ui->label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(iconSize));
        }
    }
    if (timeout == -1) { //Timeout default
        this->timeout = 5000;
    } else if (timeout > 0) { //Timeout given by notification
        this->timeout = timeout;
    } else { //No timeout
        this->timeout = -1;
    }
}

void NotificationDialog::setParams(QString appName, QString title, QString body) {
    ui->appName->setText(appName);
    ui->title->setText(title);
    ui->body->setText(body);
}

NotificationDialog::~NotificationDialog()
{
    delete ui;
}

void NotificationDialog::show() {
    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    int retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();
    closed = false;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    this->setGeometry(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height());

    //QThread::sleep(1); //Buffer for Window Manager Animations

    tPropertyAnimation *anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutBounce);
    connect(anim, &tPropertyAnimation::finished, [=](){
        this->repaint();
    });
    anim->start();

    if (timeout != -1) {
        QTimer *t = new QTimer(this);
        t->setInterval(timeout);
        t->setSingleShot(true);
        connect(t, &QTimer::timeout, [=]() {
            this->close(1);
        });
        t->start();
    }
}

void NotificationDialog::on_pushButton_clicked()
{
    this->close(2);
}

void NotificationDialog::close(int reason) {
    if (!closed) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();

        tPropertyAnimation *anim = new tPropertyAnimation(this, "geometry");
        anim->setStartValue(this->geometry());
        anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &tPropertyAnimation::finished, [=]() {
            QDialog::close();
            //delete this;
        });
        anim->start();

        emit closing(id, reason);
    }
}

void NotificationDialog::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void NotificationDialog::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void NotificationDialog::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
    event->accept();
}
