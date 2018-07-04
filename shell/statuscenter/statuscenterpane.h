#ifndef STATUSCENTERPANE_H
#define STATUSCENTERPANE_H

#include <QList>
#include "statuscenterpaneobject.h"

class StatusCenterPane {
    public:
        virtual ~StatusCenterPane() {}

        virtual QList<StatusCenterPaneObject*> availablePanes() = 0;
};

#define STATUS_CENTER_PANE_IID "org.thesuite.theshell.statuscenterpane"
Q_DECLARE_INTERFACE(StatusCenterPane, STATUS_CENTER_PANE_IID)

#endif // STATUSCENTERPANE_H
