#ifndef DBUSEVENTS_H
#define DBUSEVENTS_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QApplication>
#include <QDBusObjectPath>
#include <QDBusReply>

class DbusEvents : public QObject
{
    Q_OBJECT
public:
    explicit DbusEvents(QObject *parent = 0);

signals:

public slots:
    void LockScreen();

    void UnlockScreen();


private slots:
    void SleepingNow();

private:
    QProcess* LockScreenProcess = NULL;

};

#endif // DBUSEVENTS_H
