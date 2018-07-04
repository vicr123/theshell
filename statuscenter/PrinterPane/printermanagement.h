#ifndef PRINTERMANAGEMENT_H
#define PRINTERMANAGEMENT_H

#include <QWidget>
#include <cups/cups.h>
#include <statuscenterpaneobject.h>

namespace Ui {
    class PrinterManagement;
}

class PrinterManagement : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit PrinterManagement(QWidget *parent = 0);
        ~PrinterManagement();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();

    private:
        Ui::PrinterManagement *ui;
};

#endif // PRINTERMANAGEMENT_H
