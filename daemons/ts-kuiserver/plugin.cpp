#include "plugin.h"
#include "jobserver.h".h"

Plugin::Plugin(QObject *parent) :
    QObject(parent)
{
    panes.append(new JobServer);
}

QList<StatusCenterPaneObject*> Plugin::availablePanes() {
    return panes;
}

void Plugin::loadLanguage(QString language) {

}
