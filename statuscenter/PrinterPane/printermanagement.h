#ifndef PRINTERMANAGEMENT_H
#define PRINTERMANAGEMENT_H

#include <QWidget>
#include <QListWidgetItem>
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
        void message(QString name, QVariantList args);

    private slots:
        void on_mainMenuButton_clicked();

    private:
        Ui::PrinterManagement *ui;

        int destCount;
        cups_dest_t* dests;
};

#endif // PRINTERMANAGEMENT_H
