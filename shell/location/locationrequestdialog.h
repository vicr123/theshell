#ifndef LOCATIONREQUESTDIALOG_H
#define LOCATIONREQUESTDIALOG_H

#include <QDialog>
#include <QIcon>
#include <QLabel>

namespace Ui {
    class LocationRequestDialog;
}

class LocationRequestDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit LocationRequestDialog(QWidget *parent = 0);
        ~LocationRequestDialog();

    public slots:
        void setAppName(QString appName);
        void setIcon(QIcon icon);

    private slots:
        void on_denyButton_clicked();

        void on_allowButton_clicked();

    private:
        Ui::LocationRequestDialog *ui;

};

#endif // LOCATIONREQUESTDIALOG_H
