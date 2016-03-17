#ifndef NOTIFICATIONDIALOG_H
#define NOTIFICATIONDIALOG_H

#include <QDialog>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTimer>

namespace Ui {
class NotificationDialog;
}

class NotificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationDialog(QString title, QString body, int id, int timeout, QWidget *parent = 0);
    ~NotificationDialog();

    void show();
    void close();

    void setParams(QString title, QString body);

signals:
    void closing(int id);

private slots:
    void on_pushButton_clicked();

private:
    Ui::NotificationDialog *ui;

    int id;
    int timeout;
};

#endif // NOTIFICATIONDIALOG_H
