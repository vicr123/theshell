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
#include <QAction>
#include <QMutex>
#include <math.h>
#include "window.h"
#include "upowerdbus.h"
#include "infopanedropdown.h"
#include "systrayicons.h"
#include "fadebutton.h"
#include "FlowLayout/flowlayout.h"
#include "tutorialwindow.h"
#include "audiomanager.h"
#include "taskbarmanager.h"
#include <systemd/sd-login.h>
#include <systemd/sd-daemon.h>
#include "location/locationservices.h"
#include "screenrecorder.h"

class Menu;
class InfoPaneDropdown;
class MprisPlayer;
typedef QSharedPointer<MprisPlayer> MprisPlayerPtr;

class ChunkWatcher : public QObject
{
    Q_OBJECT
    public:
        ChunkWatcher(QWidget* chunk) : QObject(chunk) {
            chunk->installEventFilter(this);
        }

    signals:
        void visibilityChanged(bool isVisible);

    private:
        bool eventFilter(QObject* watched, QEvent* event) {
            if (event->type() == QEvent::Show) {
                emit visibilityChanged(true);
            } else if (event->type() == QEvent::Hide) {
                emit visibilityChanged(false);
            }
            return false;
        }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);
    InfoPaneDropdown* getInfoPane();
    Menu* getMenu();
    void show();

public slots:
    void openMenu();

    void doUpdate();

    void updateStruts();

    void reloadBar();

private slots:
    void on_time_clicked();

    void on_date_clicked();

    void on_volumeFrame_MouseEnter();

    void on_volumeFrame_MouseExit();

    void on_volumeSlider_sliderMoved(int position);

    void on_volumeSlider_valueChanged(int value);

    void on_brightnessFrame_MouseEnter();

    void on_brightnessFrame_MouseExit();

    void on_brightnessSlider_sliderMoved(int position);

    void on_brightnessSlider_valueChanged(int value);

    void on_volumeSlider_sliderReleased();

    void pullDownGesture();

    void setTimer(QString timer);

    void setTimerVisible(bool visible);

    void setTimerEnabled(bool enable);

    void on_timerIcon_clicked();

    void on_timer_clicked();

    void setMprisCurrentApp(QAction* action);

    void ActivateWindow();

    void reloadScreens();

    void on_desktopNext_clicked();

    void on_desktopBack_clicked();

    void on_openMenu_clicked();

    void on_time_dragging(int , int );

    void on_time_mouseReleased();

    void on_date_dragging(int , int );

    void on_date_mouseReleased();

    void on_actionNone_triggered();

    void on_actionNotifications_triggered();

    void on_actionMute_triggered();

    void updateWindow(WmWindow window);

    void deleteWindow(WmWindow window);

    void on_stopRecordingButton_clicked();

    void on_MainWindow_customContextMenuRequested(const QPoint &pos);

    void on_openMenu_customContextMenuRequested(const QPoint &pos);

    void on_openStatusCenterButton_clicked();

    void on_actionCriticalOnly_triggered();

    void lockMovement(QString reason);

    void unlockMovement(QString reason);

    void remakeBar();
    void showStatusBarProgress(bool show);

    void on_mprisSongName_clicked();
    void on_mprisForward_clicked();
    void on_mprisBack_clicked();
    void on_mprisPause_clicked();
    signals:
    void reloadBackgrounds();

private:
    Ui::MainWindow *ui;
    //QList<WmWindow> windowList;
    Menu* gatewayMenu;
    TaskbarManager* taskbarManager;

    QMap<Window, FadeButton*> buttonWindowMap;

    QSettings settings;

    int hideTop = 0;
    bool lockHide = false;
    int lockHideCount = 0;
    int attentionDemandingWindows = 0;
    int oldDesktop = 0;
    Window oldActiveWindow = 0;
    bool borderBlinkOn = true;
    bool warningAnimCreated = false;
    int warningWidth = 0;
    bool forceWindowMove = false;

    MprisPlayerPtr currentPlayer;
    QObject* mprisContextObject = nullptr;

    void closeEvent(QCloseEvent*);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void enterEvent(QEvent* event);
    void changeEvent(QEvent* event);
    void resizeEvent(QResizeEvent* event);
    bool event(QEvent* event);

    InfoPaneDropdown *infoPane;

    QPoint lastTouchPoint;
    QPoint lastTouchScreenPoint;
    int currentTouch = -1;

    tVariantAnimation* barAnim;

    int statusBarPercentage = -2;
    QTimer* statusBarProgressTimer;
    int statusBarNormalY;

    QWidget* seperatorWidget;

    QGraphicsOpacityEffect* statusBarOpacityEffect;
    bool statusBarVisible = false;
};

#endif // MAINWINDOW_H
