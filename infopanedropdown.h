#ifndef INFOPANEDROPDOWN_H
#define INFOPANEDROPDOWN_H

#include <QDialog>
#include <QResizeEvent>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <QTime>
#include "notificationdbus.h"

namespace Ui {
class InfoPaneDropdown;
}

class InfoPaneDropdown : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit InfoPaneDropdown(NotificationDBus* notificationEngine, QWidget *parent = 0);
    ~InfoPaneDropdown();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    enum dropdownType {
        Settings = -1,
        Clock = 0,
        Battery = 1,
        Network = 2,
        Notifications = 3
    };

    void show(dropdownType showWith);
    void close();

signals:
    void networkLabelChanged(QString label);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_clockLabel_clicked();

    void on_batteryLabel_clicked();

    void on_networkLabel_clicked();

    void on_notificationsLabel_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void timerTick();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

public slots:
    void getNetworks();

    void startTimer(QTime time);

private:
    Ui::InfoPaneDropdown *ui;

    NotificationDBus* notificationEngine;

    dropdownType currentDropDown = Clock;
    void changeDropDown(dropdownType changeTo);

    void resizeEvent(QResizeEvent *event);
    QTimer* timer = NULL;
    QTime timeUntilTimeout;
};

#endif // INFOPANEDROPDOWN_H
