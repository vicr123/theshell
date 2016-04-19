#ifndef CHOOSEBACKGROUND_H
#define CHOOSEBACKGROUND_H

#include <QDialog>

namespace Ui {
class ChooseBackground;
}

class ChooseBackground : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseBackground(QWidget *parent = 0);
    ~ChooseBackground();

private:
    Ui::ChooseBackground *ui;
};

#endif // CHOOSEBACKGROUND_H
