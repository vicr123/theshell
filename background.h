#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Background;
}

class Background : public QDialog
{
    Q_OBJECT

public:
    explicit Background(QWidget *parent = 0);
    ~Background();

private:
    Ui::Background *ui;
};

#endif // BACKGROUND_H
