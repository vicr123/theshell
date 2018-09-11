#ifndef PLUGIN_H
#define PLUGIN_H

#include <statuscenterpane.h>
#include "bluetoothmanagement.h"

class Plugin : public QObject, public StatusCenterPane
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID STATUS_CENTER_PANE_IID FILE "BluetoothPane.json")
        Q_INTERFACES(StatusCenterPane)

    public:
        explicit Plugin(QObject *parent = 0);

        QList<StatusCenterPaneObject*> availablePanes();
        void loadLanguage(QString language);
    private:
        QList<StatusCenterPaneObject*> panes;

        QTranslator* translator;
};

#endif // PLUGIN_H
