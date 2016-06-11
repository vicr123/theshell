#ifndef SYSTRAYICONS_H
#define SYSTRAYICONS_H


#include <QFrame>
#include <QBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QX11Info>
#include <QDebug>
#include <QTimer>
#include <QWindow>
#include <QApplication>
#include <nativeeventfilter.h>

//Xlib needs to be included LAST.
#include <X11/Xlib.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

class SysTrayIcons : public QFrame
{
    Q_OBJECT
public:
    explicit SysTrayIcons(QWidget *parent = 0);

signals:

public slots:

private slots:
    void checkForSysTrayIcons();
    void SysTrayEvent(long opcode, long data2, long data3, long data4);
};

#endif // SYSTRAYICONS_H
