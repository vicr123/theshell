#include "plugin.h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    panes.append(new Overview());
    panes.append(new OverviewSettings());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}
