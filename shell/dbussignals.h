#ifndef DBUSSIGNALS_H
#define DBUSSIGNALS_H

#include <QObject>
#include <QDBusConnection>

class DBusSignals : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.thesuite.theshell")

public:
    explicit DBusSignals(QObject *parent = 0);

signals:
    Q_SCRIPTABLE void ThemeChanged();
};

#endif // DBUSSIGNALS_H
