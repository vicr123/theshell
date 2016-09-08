#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QX11Info>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QPaintEvent>

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
class RunDialog;
}

class RunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunDialog(QWidget *parent = 0);
    ~RunDialog();

    void show();
    void close();
    void reject();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);
private slots:
    void on_cancelButton_clicked();

    void on_runButton_clicked();

    void on_command_returnPressed();

private:
    Ui::RunDialog *ui;

    void paintEvent(QPaintEvent* event);
};

#endif // RUNDIALOG_H
