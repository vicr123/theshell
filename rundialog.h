#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class RunDialog;
}

class RunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunDialog(QWidget *parent = 0);
    ~RunDialog();

private slots:
    void on_cancelButton_clicked();

    void on_runButton_clicked();

    void on_command_returnPressed();

private:
    Ui::RunDialog *ui;
};

#endif // RUNDIALOG_H
