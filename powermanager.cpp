#include "powermanager.h"

PowerManager::PowerManager(QObject *parent) : QObject(parent)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Inhibit");
    QList<QVariant> arguments;
    arguments.append("handle-power-key");
    arguments.append("theShell");
    arguments.append("theShell handles button presses");
    arguments.append("block");
    message.setArguments(arguments);

    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    qDebug() << reply.errorMessage();

    QDBusUnixFileDescriptor descriptor = reply.arguments().first().value<QDBusUnixFileDescriptor>();

    int desc = descriptor.fileDescriptor();

    inhibitLock.open(descriptor.fileDescriptor(), QFile::ReadWrite);



    //inhibitLock.close();
}

PowerManager::~PowerManager() {
    inhibitLock.close();
}
