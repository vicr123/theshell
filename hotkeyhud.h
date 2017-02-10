#ifndef HOTKEYHUD_H
#define HOTKEYHUD_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QX11Info>

#include <X11/Xlib.h>
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

namespace Ui {
class HotkeyHud;
}

class HotkeyHud : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit HotkeyHud(QWidget *parent = 0);
    ~HotkeyHud();

    void show(QIcon icon, QString control, int value);
    void show(QIcon icon, QString control, QString explanation);
    void close();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

private slots:
    void Timeout();

private:
    Ui::HotkeyHud *ui;

    void paintEvent(QPaintEvent* event);
    void show();
    bool isShowing = false;

    int value;

    QTimer* timeout = NULL;
};

#endif // HOTKEYHUD_H
