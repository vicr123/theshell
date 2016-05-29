#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QFile>
#include <QDebug>
#include <QDBusUnixFileDescriptor>
#include <stdio.h>

class PowerManager : public QObject
{
    Q_OBJECT
public:
    explicit PowerManager(QObject *parent = 0);
    ~PowerManager();

signals:

public slots:

private:
    QFile inhibitLock;
};

#endif // POWERMANAGER_H
