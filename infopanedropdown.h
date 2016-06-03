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
#include <QLabel>
#include <QMessageBox>
#include <cups/cups.h>
#include "notificationdbus.h"
#include "upowerdbus.h"
#include "endsessionwait.h"
#include "UGlobalHotkey-master/uglobalhotkeys.h"

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
        Notifications = 3,
        Print = 4
    };

    void show(dropdownType showWith);
    void close();
    bool isTimerRunning();

signals:
    void networkLabelChanged(QString label);

    void closeNotification(int id);

    void numNotificationsChanged(int notifications);

    void timerChanged(QString timer);

    void timerVisibleChanged(bool timerVisible);

    void timerEnabledChanged(bool timerEnabled);

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

    void on_printLabel_clicked();

    void on_resetButton_clicked();

    void on_TouchFeedbackSwitch_toggled(bool checked);

    void on_thewaveTTSpico2wave_clicked();

    void on_thewaveTTSfestival_clicked();

    void on_thewaveWikipediaSwitch_toggled(bool checked);

public slots:
    void getNetworks();

    void startTimer(QTime time);

    bool isQuietOn();

private:
    Ui::InfoPaneDropdown *ui;

    NotificationDBus* notificationEngine;
    UPowerDBus* powerEngine;

    bool isRedshiftOn = false;
    dropdownType currentDropDown = Clock;
    void changeDropDown(dropdownType changeTo);
    int mouseClickPoint;
    int initialPoint;
    bool mouseMovedUp = false;
    QRect dragRect;

    QMap<int, QFrame*> notificationFrames;
    QMap<QString, QFrame*> printersFrames;
    QMap<QString, QLabel*> printersStats;
    QMap<QString, QFrame*> printersStatFrames;

    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);
    QTimer* timer = NULL;
    QTimer* eventTimer;
    QTime timeUntilTimeout;

    QSettings settings;
};

#endif // INFOPANEDROPDOWN_H
