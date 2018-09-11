#include "plugin.h"
#include "daemon.h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    panes.append(new Daemon());
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {

}
