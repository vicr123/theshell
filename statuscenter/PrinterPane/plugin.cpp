#include "plugin.h"

Plugin::Plugin(QObject* parent) : QObject(parent)
{
    PrinterManagement* obj = new PrinterManagement();
    panes.append(obj);
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}
