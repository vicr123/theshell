#ifndef SEGFAULTDIALOG_H
#define SEGFAULTDIALOG_H

#include <QDialog>
#include <execinfo.h>
#include <QMessageBox>
#include <QSettings>

namespace Ui {
class SegfaultDialog;
}

class SegfaultDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SegfaultDialog(QString signal, QWidget *parent = 0);
    ~SegfaultDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::SegfaultDialog *ui;
};

#endif // SEGFAULTDIALOG_H
