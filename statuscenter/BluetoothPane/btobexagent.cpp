#include "btobexagent.h"

BTObexAgent::BTObexAgent(BluezQt::ObexManager *mgr, QObject *parent) : BluezQt::ObexAgent(parent) {
    this->mgr = mgr;

    BluezQt::PendingCall* serviceStarter = BluezQt::ObexManager::startService();
    serviceStarter->waitForFinished();

    BluezQt::InitObexManagerJob* job = mgr->init();
    connect(job, &BluezQt::InitObexManagerJob::result, [=] {
        if (job->error() == BluezQt::InitObexManagerJob::NoError) {
            mgr->registerAgent(this);
        }
    });
    job->start();
}

QDBusObjectPath BTObexAgent::objectPath() const {
    return QDBusObjectPath("/org/thesuite/tsbt");
}
