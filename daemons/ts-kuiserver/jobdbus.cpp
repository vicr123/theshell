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

JobDBus::JobDBus(QString title, QString path, QObject *parent) : QObject(parent)
{
    JobViewV2Adaptor* adaptor = new JobViewV2Adaptor(this);
    QDBusConnection::sessionBus().registerObject(path, this);

    this->path = path;
    this->t = title;
}

void JobDBus::terminate(QString errorMessage) {
    emit complete();
}

void JobDBus::setSuspended(bool suspended) {

}

void JobDBus::setTotalAmount(qulonglong amount, QString unit) {

}

void JobDBus::setProcessedAmount(qulonglong amount, QString unit) {

}

void JobDBus::setPercent(uint percent) {
    this->p = percent;
    emit update(title(), description(), this->percent());
}

void JobDBus::setSpeed(qulonglong bytesPerSecond) {

}

void JobDBus::setInfoMessage(QString message) {
    d = message;
    emit update(title(), description(), percent());
}

void JobDBus::setDescriptionField(uint number, QString name, QString value) {

}

void JobDBus::clearDescriptionField(uint number) {

}

void JobDBus::setDestUrl(QDBusVariant url) {
    qDebug() << url.variant();
}

void JobDBus::setError(uint errorCode) {
    emit complete();
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
