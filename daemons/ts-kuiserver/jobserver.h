#ifndef JOBSERVER_H
#define JOBSERVER_H

#include <QObject>
#include <QDBusObjectPath>
#include <statuscenterpaneobject.h>
#include "jobdbus.h"

class JobServer : public QObject, public StatusCenterPaneObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.JobViewServer")

    public:
        explicit JobServer(QObject *parent = nullptr);

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args = QVariantList());

    public Q_SLOTS:
        Q_SCRIPTABLE QDBusObjectPath requestView(QString appName, QString appIconName, int capabilities);
        void updateJobs();

    private:
        int currentView = 0;
        QList<JobDBus*> jobs;
};

#endif
