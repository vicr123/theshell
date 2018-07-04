#ifndef PLUGIN_H
#define PLUGIN_H

#include <statuscenterpane.h>
#include <printermanagement.h>

class Plugin : public QObject, public StatusCenterPane
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID STATUS_CENTER_PANE_IID FILE "metadata.json")
    Q_INTERFACES(StatusCenterPane)

    public:
        explicit Plugin(QObject* parent = nullptr);

        QList<StatusCenterPaneObject*> availablePanes();
    private:
        QList<StatusCenterPaneObject*> panes;
};

#endif // PLUGIN_H
