#include "mainwindow.h"
#include "background.h"
#include "loginsplash.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QProcess>
#include <QThread>
#include <QAbstractNativeEventFilter>
#include <QUrl>
#include <QMediaPlayer>
#include <QDebug>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    bool showSplash = true;

    QStringList args = a.arguments();
    args.removeFirst();
    for (QString arg : a.arguments()) {
        if (arg == "--help" || arg == "-h") {
            qDebug() << "theShell";
            qDebug() << "Usage: theshell [OPTIONS]";
            qDebug() << "  -s, --no-splash-screen       Don't show the splash screen";
            qDebug() << "  -h, --help                   Show this help output";
            return 0;
        } else if (arg == "-s" || arg == "--no-splash-screen") {
            showSplash = false;
        }
    }

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("theShell");

    QSettings settings;

    QString windowManager = settings.value("startup/WindowManagerCommand", "").toString();
    if (windowManager == "") {
        windowManager = "kwin_x11";
        settings.setValue("startup/WindowManagerCommand", windowManager);
    }

    QProcess::startDetached(windowManager);

    if (showSplash) {
        LoginSplash* splash = new LoginSplash();
        splash->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        splash->showFullScreen();
    }

    QProcess* ksuperkey = new QProcess();
    ksuperkey->start("ksuperkey -d -e \"Super_L=Alt_L|F5;Alt_R|F5\"");

    Background b;
    b.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    b.showFullScreen();

    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    w.setGeometry(screenGeometry.x() - 1, screenGeometry.y(), screenGeometry.width() + 1, w.height());
    w.show();

    QThread::sleep(1);

    return a.exec();
}

void playSound(QUrl location, bool uncompressed = false) {
    if (uncompressed) {
        QSoundEffect* sound = new QSoundEffect();
        sound->setSource(location);
        sound->play();
    } else {
        QMediaPlayer* sound = new QMediaPlayer();
        sound->setMedia(location);
        sound->play();
    }
}
