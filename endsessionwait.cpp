#include "endsessionwait.h"
#include "ui_endsessionwait.h"

EndSessionWait::EndSessionWait(shutdownType type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EndSessionWait)
{
    ui->setupUi(this);

    switch (type) {
    case powerOff:
        ui->label->setText("Power Off");
        break;
    case reboot:
        ui->label->setText("Reboot");
        break;
    case logout:
        ui->label->setText("Log out");
        break;
    case dummy:
        ui->label->setText("Dummy");
    }

    this->type = type;
}

EndSessionWait::~EndSessionWait()
{
    delete ui;
}

void EndSessionWait::showFullScreen() {
    QDialog::showFullScreen();

    if (this->type != dummy) {
        QProcess p;
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
                    WmWindow *w = new WmWindow(this);
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
        }


        for (WmWindow* window : *wlist) {
            p.start("wmctrl -c " + window->title());
            p.waitForStarted();
            while (p.state() != 0) {
                QApplication::processEvents();
            }
        }

        bool appsOpen = true;

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

void EndSessionWait::on_pushButton_clicked()
{
    QProcess p;
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
                WmWindow *w = new WmWindow(this);
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
    }

    performEndSession();
}

void EndSessionWait::performEndSession() {
    QFile(QDir::home().absolutePath() + "/.theshell.lck").remove();

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

    QGraphicsOpacityEffect *fadeEffect = new QGraphicsOpacityEffect(this);
    ui->frame->setGraphicsEffect(fadeEffect);
    QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
    a->setDuration(500);
    a->setStartValue(1);
    a->setEndValue(0);
    a->start();

    connect(a, &QPropertyAnimation::finished, [=]() {
        ui->frame->setVisible(false);
    });
}

void EndSessionWait::EndSessionNow() {
    QDBusMessage message;
    QList<QVariant> arguments;
    arguments.append(true);
    switch (type) {
    case powerOff:
        message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PowerOff");
        message.setArguments(arguments);
        QDBusConnection::systemBus().send(message);

        break;
    case reboot:
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
