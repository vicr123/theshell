#include "daemon.h"

Daemon::Daemon(QObject *parent) : QObject(parent)
{
    rotation = new AutoRotation(this);
}

QWidget* Daemon::mainWidget() {
    return nullptr;
}


QString Daemon::name() {
    return "udev";
}

StatusCenterPaneObject::StatusPaneTypes Daemon::type() {
    return None;
}

int Daemon::position() {
    return 0;
}

void Daemon::message(QString name, QVariantList args) {
    rotation->message(name, args);
}
