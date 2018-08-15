#include "plugin.h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    panes.append(new Overview());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}
