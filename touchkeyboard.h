#ifndef TOUCHKEYBOARD_H
#define TOUCHKEYBOARD_H

#include <QDialog>
#include <QDesktopWidget>
//#include <X11/Xlib.h>
//#include <X11/keysym.h>

namespace Ui {
class TouchKeyboard;
}

class TouchKeyboard : public QDialog
{
    Q_OBJECT

public:
    explicit TouchKeyboard(QWidget *parent = 0);
    ~TouchKeyboard();

private slots:
    void sendKey();
    void on_shiftButton_toggled(bool checked);

private:
    Ui::TouchKeyboard *ui;
};

#endif // TOUCHKEYBOARD_H
