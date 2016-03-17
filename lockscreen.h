#ifndef LOCKSCREEN_H
#define LOCKSCREEN_H

#include <QDialog>

namespace Ui {
class LockScreen;
}

class LockScreen : public QDialog
{
    Q_OBJECT

public:
    explicit LockScreen(QWidget *parent = 0);
    ~LockScreen();

private:
    Ui::LockScreen *ui;
};

#endif // LOCKSCREEN_H
