#ifndef NEWMEDIA_H
#define NEWMEDIA_H

#include <QDialog>
#include <QProcess>
#include <QX11Info>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>

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
class NewMedia;
}

class NewMedia : public QDialog
{
    Q_OBJECT

public:
    explicit NewMedia(QString description, QWidget *parent = 0);
    ~NewMedia();

    void show();
    void close();
    void reject();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);
private slots:
    void on_closeButton_clicked();

private:
    Ui::NewMedia *ui;

    void paintEvent(QPaintEvent* event);
};

#endif // NEWMEDIA_H
