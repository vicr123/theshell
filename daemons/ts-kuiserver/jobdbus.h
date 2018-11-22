#ifndef JOBDBUS_H
#define JOBDBUS_H

#include <QObject>
#include <QDBusVariant>

class JobDBus : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.JobViewV2")

    public:
        explicit JobDBus(QString title, QString path, QObject *parent = nullptr);

        QString title();
        QString description();
        uint percent();

    public Q_SLOTS:
        Q_SCRIPTABLE void terminate(QString errorMessage);
        Q_SCRIPTABLE void setSuspended(bool suspended);
        Q_SCRIPTABLE void setTotalAmount(qulonglong amount, QString unit);
        Q_SCRIPTABLE void setProcessedAmount(qulonglong amount, QString unit);
        Q_SCRIPTABLE void setPercent(uint percent);
        Q_SCRIPTABLE void setSpeed(qulonglong bytesPerSecond);
        Q_SCRIPTABLE void setInfoMessage(QString message);
        Q_SCRIPTABLE void setDescriptionField(uint number, QString name, QString value);
        Q_SCRIPTABLE void clearDescriptionField(uint number);
        Q_SCRIPTABLE void setDestUrl(QDBusVariant url);
        Q_SCRIPTABLE void setError(uint errorCode);

    Q_SIGNALS:
        Q_SCRIPTABLE void suspendRequested();
        Q_SCRIPTABLE void resumeRequested();
        Q_SCRIPTABLE void cancelRequested();

        void update(QString title, QString description, uint percentage);
        void complete();

    private:
        QString path;

        QString t, d;
        uint p = 0;
};

#endif // JOBDBUS_H
