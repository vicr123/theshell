#ifndef LOCATIONDAEMON_H
#define LOCATIONDAEMON_H

#include <QObject>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QMutex>
#include <QMutexLocker>
#include <tpromise.h>

struct Geolocation {
    double latitude;
    double longitude;
    double accuracy;
    double altitude;
    double speed;
    double heading;
    QString description;
    QDateTime timestamp;

    bool resolved = false;
};

class LocationDaemon : public QObject
{
        Q_OBJECT
    public:
        explicit LocationDaemon(QObject *parent = nullptr);

    signals:

    public slots:
        bool startListening();
        bool stopListening();
        tPromise<Geolocation>* singleShot();

    private slots:
        void locationUpdated();

    private:
        QDBusObjectPath geoclueClientPath;
        QDBusInterface* clientInterface = nullptr;
        int listeningTimes = 0;

        QMutex listeningMutex;
};

#endif // LOCATIONDAEMON_H
