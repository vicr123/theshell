#include "startmonitor.h"

StartMonitor::StartMonitor(QObject *parent) : QObject(parent)
{
    splash = new LoginSplash();
    splash->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);

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
    splash->show();
}

void StartMonitor::HideSplash() {
    splash->hide();
}

void StartMonitor::SplashQuestion(QString title, QString msg) {
    splash->question(title, msg);
}

void StartMonitor::SplashPrompt(QString title, QString msg) {
    splash->prompt(title, msg);
}
