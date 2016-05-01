#ifndef INFOPANEDROPDOWN_H
#define INFOPANEDROPDOWN_H

#include <QDialog>
#include <QResizeEvent>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QFrame>
#include "notificationdbus.h"
#include "upowerdbus.h"

class UPowerDBus;

namespace Ui {
class InfoPaneDropdown;
}

class InfoPaneDropdown : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit InfoPaneDropdown(NotificationDBus* notificationEngine, UPowerDBus* powerEngine, QWidget *parent = 0);
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

    void closeNotification(int id);

    void numNotificationsChanged(int notifications);

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

    void on_lineEdit_2_editingFinished();

    void on_resolutionButton_clicked();

    void on_startRedshift_timeChanged(const QTime &time);

    void on_endRedshift_timeChanged(const QTime &time);

    void on_redshiftIntensity_sliderMoved(int position);

    void on_redshiftIntensity_sliderReleased();

    void processTimer();

    void on_redshiftIntensity_valueChanged(int value);

    void newNotificationReceived(int id, QString summary, QString body, QIcon icon);

    void removeNotification(int id);

    void on_clearAllNotifications_clicked();

    void on_redshiftPause_toggled(bool checked);

    void batteryLevelChanged(int battery);

public slots:
    void getNetworks();

    void startTimer(QTime time);

private:
    Ui::InfoPaneDropdown *ui;

    NotificationDBus* notificationEngine;
    UPowerDBus* powerEngine;

    bool isRedshiftOn = false;
    dropdownType currentDropDown = Clock;
    void changeDropDown(dropdownType changeTo);

    QMap<int, QFrame*> notificationFrames;

    void resizeEvent(QResizeEvent *event);
    QTimer* timer = NULL;
    QTimer* eventTimer;
    QTime timeUntilTimeout;

    QSettings settings;
};

#endif // INFOPANEDROPDOWN_H
