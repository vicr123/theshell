#ifndef LOCATIONSERVICES_H
#define LOCATIONSERVICES_H

#include <QObject>
#include <QDBusMessage>
#include <QDBusInterface>

class LocationServices : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.GeoClue2.Agent")
    Q_PROPERTY(uint MaxAccuracyLevel READ MaxAccuracyLevel)

public:
    explicit LocationServices(QObject *parent = 0);

    uint MaxAccuracyLevel();

public Q_SLOTS:
    Q_SCRIPTABLE bool AuthorizeApp(QString desktop_id, uint req_accuracy_level, uint &allowed_accuracy_level);

signals:
    void locationUsingChanged(bool location);

public slots:
    void GeocluePropertiesChanged(QString interface, QVariantMap properties);

private:

};

#endif // LOCATIONSERVICES_H
