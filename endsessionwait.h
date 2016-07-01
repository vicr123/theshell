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
#include <QX11Info>
#include <QThread>
#include "window.h"

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
        ask,
        dummy //FOR TESTING
    };

    explicit EndSessionWait(shutdownType type, QWidget *parent = 0);
    ~EndSessionWait();

    void showFullScreen();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void EndSessionNow();
    void on_CancelAsk_clicked();

    void on_PowerOff_clicked();

    void on_Reboot_clicked();

    void on_LogOut_clicked();

    void on_Suspend_clicked();

    void on_Hibernate_clicked();

    void on_terminateApp_clicked();

    void on_exitTerminate_clicked();

    void reloadAppList();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

public slots:
    void close();

private:
    Ui::EndSessionWait *ui;

    void performEndSession();
    shutdownType type;
    bool alreadyShowing = false;
};

#endif // ENDSESSIONWAIT_H
