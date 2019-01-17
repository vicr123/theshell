/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "jobserver.h"
#include "jobviewserver_adaptor.h"

JobServer::JobServer(QObject* parent) : QObject(parent)
{
    JobViewServerAdaptor* adaptor = new JobViewServerAdaptor(this);
    KuiServerAdaptor* kuiAdaptor = new KuiServerAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/JobViewServer", this);
    QDBusConnection::sessionBus().registerService("org.kde.kuiserver");
    QDBusConnection::sessionBus().registerService("org.kde.JobViewServer");
}

QDBusObjectPath JobServer::requestView(QString appName, QString appIconName, int capabilities) {
    qDebug() << "Job View Requested for" << appName << "with capabilities" << capabilities;
    currentView++;

    QString viewPath = QString("/JobViewServer/JobView_%1").arg(currentView);
    JobDBus* job = new JobDBus(appName, viewPath);
    connect(job, &JobDBus::update, [=](QString message, QString description, uint progress) {
        updateJobs();
    });
    connect(job, &JobDBus::complete, [=] {
        sendMessage("jobDone", QVariantList() << appName << "Job Complete");
        jobs.removeOne(job);
    });

    QDBusObjectPath view;
    view.setPath(viewPath);

    jobs.append(job);

    return view;
}

void JobServer::updateJobs() {
    if (jobs.count() == 1) {
        JobDBus* job = jobs.first();
        sendMessage("jobUpdate", QVariantList() << job->title() << job->description() << job->percent());
    } else if (jobs.count() > 1) {
        int total = 0;
        int totalPercent = 0;
        for (JobDBus* job : jobs) {
            if (job->percent() > 0) {
                totalPercent += job->percent();
                total++;
            }
        }

        int percent;
        if (total != 0) {
            percent = totalPercent / total;
        } else {
            percent = 0;
        }
        sendMessage("jobUpdate", QVariantList() << tr("%n jobs running", nullptr, jobs.count()) << "" << percent);
    }
}

QWidget* JobServer::mainWidget() {
    return nullptr;
}

QString JobServer::name() {
    return "kuiserver";
}

StatusCenterPaneObject::StatusPaneTypes JobServer::type() {
    return None;
}

int JobServer::position() {
    return 0;
}

void JobServer::message(QString name, QVariantList args) {

}
