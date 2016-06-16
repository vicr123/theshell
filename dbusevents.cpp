#include "dbusevents.h"

DbusEvents::DbusEvents(QObject *parent) : QObject(parent)
{
    qint64 pid = QApplication::applicationPid();
    QDBusMessage sessionRequest = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                                 "/org/freedesktop/login1",
                                                                 "org.freedesktop.login1.Manager", "GetSessionByPid");
    sessionRequest.arguments().append(pid);
    QDBusReply<QDBusObjectPath> currentSessionPath = QDBusConnection::systemBus().call(sessionRequest);
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Lock",
                                        this, SLOT(LockScreen()));
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Unlock",
                                        this, SLOT(UnlockScreen()));

    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        "/org/freedesktop/login1",
                                        "org.freedesktop.login1.Manager", "PrepareForSleep",
                                        this, SLOT(SleepingNow()));

}

void DbusEvents::LockScreen() {
    if (LockScreenProcess == NULL) {
        LockScreenProcess = new QProcess();
        connect(LockScreenProcess, (void(QProcess::*)(int, QProcess::ExitStatus)) &QProcess::finished, [=]() {
            delete LockScreenProcess;
            LockScreenProcess = NULL;
        });
        LockScreenProcess->start("/usr/lib/tsscreenlock");
    }
}

void DbusEvents::UnlockScreen() {
    if (LockScreenProcess != NULL) {
        LockScreenProcess->terminate();
    }
}

void DbusEvents::SleepingNow() {
    LockScreen();
}
