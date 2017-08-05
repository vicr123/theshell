/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
#include <QFileDialog>
#include <QMediaPlaylist>
#include <QCalendarWidget>
#include <cups/cups.h>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFontComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include "animatedstackedwidget.h"
#include "notificationdbus.h"
#include "upowerdbus.h"
#include "endsessionwait.h"
#include "audiomanager.h"
#include "nativeeventfilter.h"
#include <QScrollBar>
#include <sys/sysinfo.h>
#include <QSysInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimeEdit>
#include <QStyledItemDelegate>
#include <QFuture>
#include <QFutureWatcher>

class UPowerDBus;

using namespace QtCharts;

namespace Ui {
class InfoPaneDropdown;
}

class InfoPaneDropdown : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit InfoPaneDropdown(WId MainWindowId, QWidget *parent = 0);
    ~InfoPaneDropdown();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    enum dropdownType {
        Settings = -1,
        Clock = 0,
        Battery = 1,
        Network = 2,
        Notifications = 3,
        KDEConnect = 4,
        Print = 5
    };

    enum networkAvailability {
        Unspecified = 0,
        Ok = 1,
        BehindPortal = 2
    };

    void show(dropdownType showWith);
    void dragDown(dropdownType showWith, int y);
    void close();
    bool isTimerRunning();
    void completeDragDown();

signals:
    void networkLabelChanged(QString label, int signalStrength);
    void closeNotification(int id);
    void numNotificationsChanged(int notifications);
    void timerChanged(QString timer);
    void timerVisibleChanged(bool timerVisible);
    void timerEnabledChanged(bool timerEnabled);
    void notificationsSilencedChanged(bool silenced);
    void batteryStretchChanged(bool isOn);
    void flightModeChanged(bool flight);
    void updateStrutsSignal();

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

    void on_printLabel_clicked();

    void on_resetButton_clicked();

    void on_TouchFeedbackSwitch_toggled(bool checked);

    void on_thewaveTTSpico2wave_clicked();

    void on_thewaveTTSfestival_clicked();

    void on_thewaveWikipediaSwitch_toggled(bool checked);

    void on_thewaveTTSespeak_clicked();

    void on_thewaveOffensiveSwitch_toggled(bool checked);

    void on_theWaveName_textEdited(const QString &arg1);

    void on_brightnessSlider_sliderMoved(int position);

    void on_brightnessSlider_valueChanged(int value);

    void on_settingsList_currentRowChanged(int currentRow);

    void on_settingsTabs_currentChanged(int arg1);

    void on_lockScreenBackgroundBrowse_clicked();

    void on_lockScreenBackground_textEdited(const QString &arg1);

    void notificationClosed(uint id, uint reason);

    void newNetworkDevice(QDBusObjectPath device);

    void on_networkConnect_clicked();

    void on_networkList_currentItemChanged(QListWidgetItem *current, QListWidgetItem*);

    void on_TextSwitch_toggled(bool checked);

    void on_windowManager_textEdited(const QString &arg1);

    void on_barDesktopsSwitch_toggled(bool checked);

    void on_thewaveTTSsilent_clicked();

    void on_theWaveSwitch_toggled(bool checked);

    void bluetoothEnabledChanged();

    void on_BluetoothSwitch_toggled(bool checked);

    void on_SuperkeyGatewaySwitch_toggled(bool checked);

    void updateSysInfo();

    void on_QuietCheck_toggled(bool checked);

    void on_kdeconnectLabel_clicked();

    void updateKdeconnect();

    void on_startKdeconnect_clicked();

    void on_endSessionConfirmFullScreen_toggled(bool checked);

    void on_endSessionConfirmInMenu_toggled(bool checked);

    void on_pageStack_switchingFrame(int switchTo);

    void on_showNotificationsContents_toggled(bool checked);

    void on_showNotificationsOnly_toggled(bool checked);

    void on_showNotificationsNo_toggled(bool checked);

    void on_stopwatchStart_clicked();

    void on_stopwatchReset_clicked();

    void on_calendarTodayButton_clicked();

    void on_MediaSwitch_toggled(bool checked);

    void on_lightColorThemeRadio_toggled(bool checked);

    void on_darkColorThemeRadio_toggled(bool checked);

    void on_themeButtonColor_currentIndexChanged(int index);

    void on_systemFont_currentFontChanged(const QFont &f);

    void on_locateDeviceButton_clicked();

    void on_pingDeviceButton_clicked();

    void updateTimers();

    void updateBatteryChart();

    void on_batteryChartUpdateButton_clicked();

    void on_batteryChartShowProjected_toggled(bool checked);

    void on_upArrow_clicked();

    void on_PowerStretchSwitch_toggled(bool checked);

    void doNetworkCheck();

    void on_notificationSoundBox_currentIndexChanged(int index);

    void setupUsersSettingsPane();

    void on_userSettingsNextButton_clicked();

    void on_userSettingsCancelButton_clicked();

    void on_userSettingsApplyButton_clicked();

    void on_userSettingsFullName_textEdited(const QString &arg1);

    void on_userSettingsDeleteUser_clicked();

    void on_userSettingsCancelDeleteUser_clicked();

    void on_userSettingsDeleteUserOnly_clicked();

    void on_userSettingsDeleteUserAndData_clicked();

    void setupDateTimeSettingsPane();

    void launchDateTimeService();

    void on_dateTimeSetDateTimeButton_clicked();

    void on_DateTimeNTPSwitch_toggled(bool checked);

    void on_localeList_currentRowChanged(int currentRow);

    void on_StatusBarSwitch_toggled(bool checked);

    void on_TouchInputSwitch_toggled(bool checked);

    void on_quietModeSound_clicked();

    void on_quietModeNotification_clicked();

    void on_quietModeMute_clicked();

    void on_ReminderCancel_clicked();

    void on_ReminderNew_clicked();

    void on_ReminderCreate_clicked();

    void on_SuspendLockScreen_toggled(bool checked);

    void on_BatteryChargeScrollBar_valueChanged(int value);

    void on_chargeGraphButton_clicked();

    void on_rateGraphButton_clicked();

    void on_appsGraphButton_clicked();

    void on_LargeTextSwitch_toggled(bool checked);

    void on_HighContrastSwitch_toggled(bool checked);

    void on_systemAnimationsAccessibilitySwitch_toggled(bool checked);

    void on_CapsNumLockBellSwitch_toggled(bool checked);

    void DBusServiceRegistered(QString serviceName);

    void DBusServiceUnregistered(QString serviceName);

    void on_FlightSwitch_toggled(bool checked);

    void on_TwentyFourHourSwitch_toggled(bool checked);

    void on_systemIconTheme_currentIndexChanged(int index);

    void on_ReminderDelete_clicked();

    void on_RemindersStackedWidget_currentChanged(int arg1);

    void on_ReminderDeleteCancel_clicked();

    void on_ReminderDeleteButton_clicked();

    void on_AttenuateSwitch_toggled(bool checked);

    void on_BarOnBottom_toggled(bool checked);

    void updateStruts();

public slots:
    void getNetworks();

    void startTimer(QTime time);

    //Switches for theWave
    void on_WifiSwitch_toggled(bool checked);

private:
    Ui::InfoPaneDropdown *ui;

    bool isRedshiftOn = false;
    dropdownType currentDropDown = Clock;
    void changeDropDown(dropdownType changeTo, bool doAnimation = true);
    int mouseClickPoint;
    int initialPoint;
    bool mouseMovedUp = false;
    QRect dragRect;

    QMap<int, QFrame*> notificationFrames;
    QMap<QString, QFrame*> printersFrames;
    QMap<QString, QLabel*> printersStats;
    QMap<QString, QFrame*> printersStatFrames;
    QMap<QString, QString> connectedNetworks;

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QTimer* timer = NULL;
    int timerNotificationId = 0;
    QTimer* eventTimer;
    QTime timeUntilTimeout;
    QTime startTime;
    void reject();

    QTimer* networkCheckTimer;
    networkAvailability networkOk = Ok;

    QTime stopwatchTime;
    int stopwatchTimeAdd = 0;
    bool stopwatchRunning = false;

    QSettings settings;
    QSettings* lockScreenSettings = new QSettings("theSuite", "tsscreenlock", this);
    QSettings* themeSettings = new QSettings("theSuite", "ts-qtplatform");

    QString editingUserPath;

    bool networkListUpdating = false;

    QMediaPlayer* ringtone;

    QChart* batteryChart;
    QDateTimeAxis* xAxis;
    bool chartScrolling = false;
    int startValue;

    int previousDragY;
    WId MainWindowId;
};

class RemindersListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    RemindersListModel(QObject *parent = 0);
    ~RemindersListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateData();

private:
    QSettings* RemindersData;
};

class RemindersDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RemindersDelegate(QWidget *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // INFOPANEDROPDOWN_H
