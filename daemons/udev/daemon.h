#ifndef DAEMON_H
#define DAEMON_H

#include <QObject>
#include <QVariant>
#include <statuscenterpaneobject.h>
#include "autorotation.h"

class AutoRotation;

class Daemon : public QObject, public StatusCenterPaneObject
{
    Q_OBJECT
public:
    explicit Daemon(QObject *parent = nullptr);

    QWidget* mainWidget();
    QString name();
    StatusPaneTypes type();
    int position();
    void message(QString name, QVariantList args = QVariantList());

signals:

public slots:

private:
    AutoRotation* rotation;
};

#endif // DAEMON_H
