#ifndef LOGINSPLASH_H
#define LOGINSPLASH_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class LoginSplash;
}

class LoginSplash : public QDialog
{
    Q_OBJECT

public:
    explicit LoginSplash(QWidget *parent = 0);
    ~LoginSplash();

private:
    Ui::LoginSplash *ui;
};

#endif // LOGINSPLASH_H
