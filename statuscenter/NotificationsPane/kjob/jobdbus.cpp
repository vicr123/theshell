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

#include "jobdbus.h"
#include "jobviewv2_adaptor.h"

#include "notificationsWidget/notificationswidget.h"
#include "jobviewwidget.h"

JobDBus::JobDBus(NotificationsWidget* widget, QString title, QString icon, QString path, int capabilities, QObject *parent) : QObject(parent)
{
    JobViewV2Adaptor* adaptor = new JobViewV2Adaptor(this);
    QDBusConnection::sessionBus().registerObject(path, this);

    this->path = path;
    this->t = title;

    this->view = new JobViewWidget(title, icon, capabilities);
    widget->addJobView(this->view);

    connect(view, &JobViewWidget::terminate, [=] {
        emit cancelRequested();
    });
    connect(view, &JobViewWidget::suspend, [=] {
        if (suspended) {
            return resumeRequested();
        } else {
            return suspendRequested();
        }
    });
}

void JobDBus::terminate(QString errorMessage) {
    emit complete();
    view->deleteLater();
}

void JobDBus::setSuspended(bool suspended) {
    this->suspended = suspended;
    view->setSuspended(suspended);
}

void JobDBus::setTotalAmount(qulonglong amount, QString unit) {

}

void JobDBus::setProcessedAmount(qulonglong amount, QString unit) {

}

void JobDBus::setPercent(uint percent) {
    this->p = percent;
    emit update(title(), description(), this->percent());
    view->setPercent(percent);
}

void JobDBus::setSpeed(qulonglong bytesPerSecond) {

}

void JobDBus::setInfoMessage(QString message) {
    d = message;
    emit update(title(), description(), percent());
    view->setInfoMessage(message);
}

void JobDBus::setDescriptionField(uint number, QString name, QString value) {
    view->setDescriptionField(number, name, value);
}

void JobDBus::clearDescriptionField(uint number) {
    view->clearDescriptionField(number);
}

void JobDBus::setDestUrl(QDBusVariant url) {
    qDebug() << url.variant();
}

void JobDBus::setError(uint errorCode) {
    emit complete();
    view->deleteLater();
}

QString JobDBus::title() {
    return t;
}

QString JobDBus::description() {
    return d;
}

uint JobDBus::percent() {
    return p;
}
