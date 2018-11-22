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
