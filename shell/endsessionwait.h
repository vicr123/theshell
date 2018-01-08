/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#ifndef ENDSESSIONWAIT_H
#define ENDSESSIONWAIT_H

#include <X11/Xlib.h>
#define Bool int
#define Status int
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#undef Status
#undef Bool
#undef None
#undef FocusIn
#undef FontChange
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusOut
#undef CursorShape
#undef Unsorted

#include <QDialog>
#include <QProcess>
#include <QMediaPlayer>
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFile>
#include <QDir>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QX11Info>
#include <QThread>
#include <QProgressBar>
#include <QLabel>
#include <QTimer>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QLayout>
#include <QMouseEvent>
#include "window.h"
#include "tpropertyanimation.h"
#include <QToolButton>

#include <signal.h>


namespace Ui {
class EndSessionWait;
}

class EndSessionWait : public QDialog
{
    Q_OBJECT

public:
    enum shutdownType {
        powerOff,
        reboot,
        logout,

        slideOff, //For tablet mode

        //These should not be shown.
        suspend,
        hibernate,
        ask,
        screenOff,
        dummy //FOR TESTING
    };

    explicit EndSessionWait(shutdownType type, QWidget *parent = 0);
    ~EndSessionWait();

    void showFullScreen();
    void reject();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void EndSessionNow();
    void on_CancelAsk_clicked();

    void on_PowerOff_clicked();

    void on_Reboot_clicked();

    void on_LogOut_clicked();

    void on_Suspend_clicked();

    void on_terminateApp_clicked();

    void on_exitTerminate_clicked();

    void reloadAppList();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_DummyExit_clicked();

public slots:
    void close();

private:
    Ui::EndSessionWait *ui;

    void performEndSession();
    shutdownType type;
    bool alreadyShowing = false;

    QVariantAnimation* powerOffTimer;

    int pressLocation;

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    bool eventFilter(QObject *obj, QEvent *eve);
};

#endif // ENDSESSIONWAIT_H
