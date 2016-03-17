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
#include "window.h"
#include "menu.h"
#include "notificationdbus.h"
#include "upowerdbus.h"
#include "UGlobalHotkey-master/uglobalhotkeys.h"
#include "infopanedropdown.h"
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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void reloadWindows();
    void activateWindow(QString windowTitle);

    void on_time_clicked();

private:
    Ui::MainWindow *ui;
    QList<WmWindow*> *windowList;

    int hideTop = 0;
    bool hiding = false;

    void keyPressEvent(QKeyEvent* event);
    void grabKeys();
};

#endif // MAINWINDOW_H
