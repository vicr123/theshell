#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QList>
#include <QProcess>
#include <QSignalMapper>
#include <QDesktopWidget>
#include <QDateTime>
#include <QtDBus/QDBusConnection>
#include <QMessageBox>
#include <QSettings>
#include <QSoundEffect>
#include <QPainter>
#include <QMenu>
#include <math.h>
#include "window.h"
#include "menu.h"
#include "notificationdbus.h"
#include "upowerdbus.h"
#include "UGlobalHotkey-master/uglobalhotkeys.h"
#include "infopanedropdown.h"
#include "touchkeyboard.h"
#include "powermanager.h"
#include "systrayicons.h"
#include "fadebutton.h"
#include "FlowLayout/flowlayout.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <QX11Info>
#include <systemd/sd-login.h>
#include <systemd/sd-daemon.h>

#undef Bool

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);
    InfoPaneDropdown* getInfoPane();
    void show();

private slots:
    void on_pushButton_clicked();
    void reloadWindows();
    void activateWindow(QString windowTitle);

    void on_time_clicked();

    void on_date_clicked();

    void on_pushButton_2_clicked();

    void internetLabelChanged(QString display);

    void on_networkLabel_clicked();

    void on_notifications_clicked();

    void on_batteryLabel_clicked();

    void on_volumeFrame_MouseEnter();

    void on_volumeFrame_MouseExit();

    void on_volumeSlider_sliderMoved(int position);

    void on_volumeSlider_valueChanged(int value);

    void on_brightnessFrame_MouseEnter();

    void on_brightnessFrame_MouseExit();

    void on_brightnessSlider_sliderMoved(int position);

    void on_brightnessSlider_valueChanged(int value);

    void on_volumeSlider_sliderReleased();

    void numNotificationsChanged(int notifications);

    void pullDownGesture();

    void on_pushButton_4_clicked();

    void setTimer(QString timer);

    void setTimerVisible(bool visible);

    void setTimerEnabled(bool enable);

    void on_timerIcon_clicked();

    void on_timer_clicked();

    void DBusNewService(QString name);

    void on_mprisPause_clicked();

    void on_mprisBack_clicked();

    void on_pushButton_3_clicked();

    void on_mprisSongName_clicked();

    void ActivateWindow();

    void reloadScreens();

    void on_desktopNext_clicked();

    void on_desktopBack_clicked();

private:
    Ui::MainWindow *ui;
    QList<WmWindow*> *windowList;

    int hideTop = 0;
    bool hiding = false;
    bool lockHide = false;
    int attentionDemandingWindows = 0;
    int oldDesktop = 0;
    Window oldActiveWindow = 0;
    bool borderBlinkOn = true;
    bool warningAnimCreated = false;
    int warningWidth = 0;
    bool forceWindowMove = false;

    QString mprisCurrentAppName = "";
    QStringList mprisDetectedApps;

    void closeEvent(QCloseEvent*);
    void paintEvent(QPaintEvent *event);

    InfoPaneDropdown *infoPane;

    void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0,
                                 long data2 = 0, long data3 = 0, long data4 = 0);
};

#endif // MAINWINDOW_H
