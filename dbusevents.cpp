#include "dbusevents.h"

DbusEvents::DbusEvents(QObject *parent) : QObject(parent)
{
    qint64 pid = QApplication::applicationPid(); //Get this PID
    QDBusMessage sessionRequest = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                                 "/org/freedesktop/login1",
                                                                 "org.freedesktop.login1.Manager", "GetSessionByPid"); //Get this session
    sessionRequest.arguments().append(pid);
    QDBusReply<QDBusObjectPath> currentSessionPath = QDBusConnection::systemBus().call(sessionRequest); //Get this session
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Lock",
                                        this, SLOT(LockScreen())); //Register Lock
    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        currentSessionPath.value().path(),
                                        "org.freedesktop.login1.Session", "Unlock",
                                        this, SLOT(UnlockScreen())); //Register Unlock

    QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                        "/org/freedesktop/login1",
                                        "org.freedesktop.login1.Manager", "PrepareForSleep",
                                        this, SLOT(SleepingNow())); //Register Sleep

}

void DbusEvents::LockScreen() {
    if (LockScreenProcess == NULL) {
        LockScreenProcess = new QProcess();
        connect(LockScreenProcess, (void(QProcess::*)(int, QProcess::ExitStatus)) &QProcess::finished, [=]() {
            delete LockScreenProcess;
            LockScreenProcess = NULL; //Delete Process
        });
        LockScreenProcess->start("/usr/lib/tsscreenlock"); //Lock Screen
    }
}

void DbusEvents::UnlockScreen() {
    if (LockScreenProcess != NULL) {
        LockScreenProcess->terminate(); //Kill Process
        //Process will be deleted in the finished() signal
    }
}

void DbusEvents::SleepingNow() {
    LockScreen();
}
