#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <QX11Info>
#include <QProcess>
#include <QTime>
#include <math.h>
#include <QIcon>
#include "hotkeyhud.h"
#include "endsessionwait.h"
#include "dbusevents.h"
#include "rundialog.h"
#include <QDBusUnixFileDescriptor>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDebug>
#include <QMessageBox>
#include <QSoundEffect>
#include "mainwindow.h"

#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject* parent = 0);
    ~NativeEventFilter();

signals:
    void SysTrayEvent(long opcode, long data2, long data3, long data4);
public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    QTime lastPress;
    HotkeyHud* Hotkeys;

    bool isEndSessionBoxShowing = false;
};

#endif // NATIVEEVENTFILTER_H
