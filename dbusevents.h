#ifndef DBUSEVENTS_H
#define DBUSEVENTS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QApplication>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDebug>
#include <QDBusInterface>
#include <QSettings>
#include "newmedia.h"

class DbusEvents : public QObject
{
    Q_OBJECT
public:
    explicit DbusEvents(QObject *parent = 0);

signals:

public slots:
    void LockScreen();

    void UnlockScreen();

    void NewUdisksInterface(QDBusObjectPath path);

private slots:
    void SleepingNow();

private:
    QProcess* LockScreenProcess = NULL;

    QSettings settings;
};

#endif // DBUSEVENTS_H
