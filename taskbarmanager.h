#ifndef TASKBARMANAGER_H
#define TASKBARMANAGER_H

#include <QObject>
#include <QMap>
#include <QX11Info>
#include <QApplication>
#include "window.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#undef Bool

class TaskbarManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskbarManager(QObject *parent = nullptr);

    QList<WmWindow> Windows();
signals:
    void windowsChanged();
    void updateWindow(WmWindow changedWindow);
    void deleteWindow(WmWindow closedWindow);

public slots:
    void ReloadWindows();

private slots:
    void updateInternalWindow(Window window);

private:
    QMap<Window, WmWindow> knownWindows;
};

#endif // TASKBARMANAGER_H
