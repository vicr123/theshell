#include "plugin.h"


Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    panes.append(new BluetoothManagement());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}
