#include "mainwindow.h"
#include "background.h"
#include "nativeeventfilter.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QProcess>
#include <QThread>
#include <QAbstractNativeEventFilter>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QProcess::startDetached("kwin_x11");

    Background b;
    b.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
    b.showMaximized();
    //QProcess::execute("wmctrl -r " + b.windowTitle() + " -b add,below");


    MainWindow w;
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    w.setGeometry(screenGeometry.x() - 1, screenGeometry.y(), screenGeometry.width() + 1, w.height());
    w.show();

    QThread::sleep(1);

    //QProcess::execute("wmctrl -r " + w.windowTitle() + " -b add,sticky");

    NativeEventFilter *filter = new NativeEventFilter();
    a.installNativeEventFilter(filter);

    return a.exec();
}
