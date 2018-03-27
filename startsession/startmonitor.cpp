#include "startmonitor.h"

StartMonitor::StartMonitor(QObject *parent) : QObject(parent)
{
    splash = new LoginSplash();
    splash->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(splash, SIGNAL(response(QString)), this, SIGNAL(questionResponse(QString)));
}

bool StartMonitor::started() {
    return s;
}

void StartMonitor::MarkStarted() {
    s = true;
    splash->hide();
}

void StartMonitor::MarkNotStarted() {
    s = false;
}

void StartMonitor::ShowSplash() {
    splash->showFullScreen();
}

void StartMonitor::HideSplash() {
    splash->hide();
}

void StartMonitor::SplashQuestion(QString title, QString msg) {
    splash->question(title, msg);
}
