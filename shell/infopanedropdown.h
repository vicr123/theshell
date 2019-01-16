/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
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
#include <QTimer>
#include <QTime>
#include <QMap>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QMediaPlaylist>
#include <QCalendarWidget>
//#include <cups/cups.h>
#include <crypt.h>
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
#include "upowerdbus.h"
#include "endsessionwait.h"
#include "audiomanager.h"
#include "nativeeventfilter.h"
#include "dbussignals.h"
#include <QScrollBar>
#include <sys/sysinfo.h>
#include <QSysInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimeEdit>
#include <QStyledItemDelegate>
#include <QFuture>
#include <QFutureWatcher>
#include <ttoast.h>
#include <QDBusServiceWatcher>
#include <QGeoPositionInfoSource>
#include "apps/appslistmodel.h"
#include <QSpinBox>
#include <polkit-qt5-1/PolkitQt1/Authority>
#include <QPluginLoader>
#include "statuscenter/statuscenterpane.h"
#include "clickablelabel.h"
#include "switch.h"

class UPowerDBus;

using namespace QtCharts;

class InfoPaneDropdownPrivate;

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
            KDEConnect = 3 //,
            //Print = 5
        };

        enum networkAvailability {
            Unspecified = 0,
            Ok = 1,
            BehindPortal = 2
        };

        void show(dropdownType showWith);
        void showNoAnimation();
        void dragDown(dropdownType showWith, int y);
        void close();
        void completeDragDown();

    signals:
        void networkLabelChanged(QString label, QIcon icon);
        void closeNotification(int id);
        void timerEnabledChanged(bool timerEnabled);
        void batteryStretchChanged(bool isOn);
        void flightModeChanged(bool flight);
        void updateStrutsSignal();
        void updateBarSignal();
        void redshiftEnabledChanged(bool enabled);
        void keyboardLayoutChanged(QString code);
        void newKeyboardLayoutMenuAvailable(QMenu* menu);
        void statusBarProgress(QString title, QString description, int progress);
        void statusBarProgressFinished(QString title, QString description);

    private slots:
        void on_pushButton_clicked();

        void on_pushButton_5_clicked();

        void on_pushButton_6_clicked();

        void on_clockLabel_clicked();

        void on_batteryLabel_clicked();

        void on_networkLabel_clicked();

        void on_pushButton_7_clicked();

        void on_resolutionButton_clicked();

        void on_startRedshift_timeChanged(const QTime &time);

        void on_endRedshift_timeChanged(const QTime &time);

        void on_redshiftIntensity_sliderMoved(int position);

        void on_redshiftIntensity_sliderReleased();

        void processTimer();

        void on_redshiftIntensity_valueChanged(int value);

        void on_redshiftPause_toggled(bool checked);

        void on_resetButton_clicked();

        void on_TouchFeedbackSwitch_toggled(bool checked);

        void on_brightnessSlider_sliderMoved(int position);

        void on_brightnessSlider_valueChanged(int value);

        void on_settingsList_currentRowChanged(int currentRow);

        void on_settingsTabs_currentChanged(int arg1);

        void on_lockScreenBackgroundBrowse_clicked();

        void on_lockScreenBackground_textEdited(const QString &arg1);

        void newNetworkDevice(QDBusObjectPath device);

        void on_TextSwitch_toggled(bool checked);

        void on_windowManager_textEdited(const QString &arg1);

        void on_barDesktopsSwitch_toggled(bool checked);

        void on_SuperkeyGatewaySwitch_toggled(bool checked);

        void updateSysInfo();

        void on_kdeconnectLabel_clicked();

        void on_endSessionConfirmFullScreen_toggled(bool checked);

        void on_endSessionConfirmInMenu_toggled(bool checked);

        void on_pageStack_switchingFrame(int switchTo);

        void on_showNotificationsContents_toggled(bool checked);

        void on_showNotificationsOnly_toggled(bool checked);

        void on_showNotificationsNo_toggled(bool checked);

        void on_MediaSwitch_toggled(bool checked);

        void on_lightColorThemeRadio_toggled(bool checked);

        void on_darkColorThemeRadio_toggled(bool checked);

        void on_themeButtonColor_currentIndexChanged(int index);

        void on_systemFont_currentFontChanged(const QFont &f);

        void updateBatteryChart();

        void on_batteryChartUpdateButton_clicked();

        void on_batteryChartShowProjected_toggled(bool checked);

        void on_upArrow_clicked();

        void on_PowerStretchSwitch_toggled(bool checked);

        void doNetworkCheck();

        void on_notificationSoundBox_currentIndexChanged(int index);

        void setupUsersSettingsPane();

        void setupLocationSettingsPane();

        void on_userSettingsNextButton_clicked();

        void on_userSettingsCancelButton_clicked();

        void on_userSettingsApplyButton_clicked();

        void on_userSettingsFullName_textEdited(const QString &arg1);

        void on_userSettingsDeleteUser_clicked();

        void on_userSettingsCancelDeleteUser_clicked();

        void on_userSettingsDeleteUserOnly_clicked();

        void on_userSettingsDeleteUserAndData_clicked();

        void setupDateTimeSettingsPane();

        void setupNotificationsSettingsPane();

        void launchDateTimeService();

        void on_dateTimeSetDateTimeButton_clicked();

        void on_DateTimeNTPSwitch_toggled(bool checked);

        void on_localeList_currentRowChanged(int currentRow);

        void on_StatusBarSwitch_toggled(bool checked);

        void on_TouchInputSwitch_toggled(bool checked);

        void on_SuspendLockScreen_toggled(bool checked);

        void on_BatteryChargeScrollBar_valueChanged(int value);

        void on_chargeGraphButton_clicked();

        void on_rateGraphButton_clicked();

        void on_appsGraphButton_clicked();

        void on_LargeTextSwitch_toggled(bool checked);

        void on_HighContrastSwitch_toggled(bool checked);

        void on_systemAnimationsAccessibilitySwitch_toggled(bool checked);

        void on_CapsNumLockBellSwitch_toggled(bool checked);

        void on_FlightSwitch_toggled(bool checked);

        void on_TwentyFourHourSwitch_toggled(bool checked);

        void on_systemIconTheme_currentIndexChanged(int index);

        void on_AttenuateSwitch_toggled(bool checked);

        void on_BarOnBottom_toggled(bool checked);

        void on_systemWidgetTheme_currentIndexChanged(int index);

        void resetStyle();

        void on_decorativeColorThemeRadio_toggled(bool checked);

        void on_SoundFeedbackSoundSwitch_toggled(bool checked);

        void on_VolumeOverdriveSwitch_toggled(bool checked);

        void updateAccentColourBox();

        void on_dpi100_toggled(bool checked);

        void on_dpi150_toggled(bool checked);

        void on_dpi200_toggled(bool checked);

        void on_dpi300_toggled(bool checked);

        void on_AutoShowBarSwitch_toggled(bool checked);

        void on_userSettingsStandardAccount_toggled(bool checked);

        void on_userSettingsAdminAccount_toggled(bool checked);

        void updateAutostart();

        void on_autostartList_itemChanged(QListWidgetItem *item);

        void on_backAutoStartApps_clicked();

        void on_pushButton_4_clicked();

        void on_backAutoStartNewApp_clicked();

        void on_autostartAppList_clicked(const QModelIndex &index);

        void on_enterCommandAutoStartApps_clicked();

        void on_addAutostartApp_clicked();

        void on_redshiftSwitch_toggled(bool checked);

        void on_grayColorThemeRadio_toggled(bool checked);

        void on_AppNotifications_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

        void on_appAllowNotifications_toggled(bool checked);

        void on_appAllowSounds_toggled(bool checked);

        void on_appAllowPopup_toggled(bool checked);

        void on_appBypassQuiet_toggled(bool checked);

        void on_SetSystemTimezoneButton_clicked();

        void on_backTimezone_clicked();

        void on_setTimezoneButton_clicked();

        void on_timezoneList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

        void on_timezoneCityList_currentRowChanged(int currentRow);

        void on_batteryScreenOff_valueChanged(int value);

        void on_batterySuspend_valueChanged(int value);

        void on_powerScreenOff_valueChanged(int value);

        void on_powerSuspend_valueChanged(int value);

        void on_removeAutostartButton_clicked();

        void on_resetDeviceButton_clicked();

        void on_sunlightRedshift_toggled(bool checked);

        void updateRedshiftTime();

        void on_systemGTK3Theme_currentIndexChanged(int index);

        void on_systemFontSize_valueChanged(int arg1);

        void on_systemGTK3Font_currentFontChanged(const QFont &f);

        void on_systemGTK3FontSize_valueChanged(int arg1);

        void on_useSystemFontForGTKButton_clicked();

        void setHeaderColour(QColor col);

        void on_EmphasiseAppSwitch_toggled(bool checked);

        void on_CompactBarSwitch_toggled(bool checked);

        void on_blackColorThemeRadio_toggled(bool checked);

        void on_allowGeoclueAgent_clicked();

        void on_LocationMasterSwitch_toggled(bool checked);

        void on_LocationAppsList_itemChanged(QListWidgetItem *item);

        void on_backInput_clicked();

        void on_addLayout_clicked();

        void on_addKeyboardLayout_clicked();

        void on_removeLayout_clicked();

        void on_selectedLayouts_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

        void on_availableKeyboardLayouts_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

        void KeyboardLayoutsMoved();

        void on_moveLayoutDown_clicked();

        void on_moveLayoutUp_clicked();

        void loadNewKeyboardLayoutMenu();

        void setKeyboardLayout(QString layout);

        void on_setupMousePassword_clicked();

        void on_removeMousePassword_clicked();

        void on_MousePasswordSetup_exit();

        void on_websiteButton_clicked();

        void on_bugButton_clicked();

        void on_distroWebpage_clicked();

        void on_distroSupport_clicked();

        void on_sourcesButton_clicked();

        void pluginMessage(QString message, QVariantList args, StatusCenterPaneObject* caller);

        QVariant pluginProperty(QString key);

        void on_settingsList_itemActivated(QListWidgetItem *item);

        void on_powerSuspendNormally_toggled(bool checked);

        void on_powerSuspendTurnOffScreen_toggled(bool checked);

        void on_powerSuspendHibernate_toggled(bool checked);

        void on_powerButtonPressed_currentIndexChanged(int index);

    public slots:
        void getNetworks();

        void on_WifiSwitch_toggled(bool checked);
        void updateStruts();
        void changeSettingsPane(int pane);

        QString setNextKeyboardLayout();

    private:
        Ui::InfoPaneDropdown *ui;
        InfoPaneDropdownPrivate* d;

        void changeDropDown(dropdownType changeTo, bool doAnimation = true);
        void changeDropDown(QWidget* changeTo, ClickableLabel* label, bool doAnimation = true);
        void reject();

        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent* event);
        void changeEvent(QEvent* event);
        void paintEvent(QPaintEvent* event);
        bool eventFilter(QObject *obj, QEvent *e);
};

class InfoPaneNotOnTopLocker {
    public:
        InfoPaneNotOnTopLocker(InfoPaneDropdown* infoPane);
        ~InfoPaneNotOnTopLocker();

    private:
        InfoPaneDropdown* infoPane;
};

#endif // INFOPANEDROPDOWN_H
