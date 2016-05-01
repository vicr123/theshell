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
#include <math.h>
#include "window.h"
#include "menu.h"
#include "notificationdbus.h"
#include "upowerdbus.h"
#include "UGlobalHotkey-master/uglobalhotkeys.h"
#include "infopanedropdown.h"
#include "thewave.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <QX11Info>

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

private slots:
    void on_pushButton_clicked();
    void reloadWindows();
    void activateWindow(QString windowTitle);

    void on_time_clicked();

    void openingApp(QString AppName, QIcon AppIcon);

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

private:
    Ui::MainWindow *ui;
    QList<WmWindow*> *windowList;

    int hideTop = 0;
    bool hiding = false;
    bool lockHide = false;

    void closeEvent(QCloseEvent*);

    InfoPaneDropdown *infoPane;

    void paintEvent(QPaintEvent *event);
};

#endif // MAINWINDOW_H
