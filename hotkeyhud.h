#ifndef HOTKEYHUD_H
#define HOTKEYHUD_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QDesktopWidget>
#include <QPaintEvent>

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

    QTimer* timeout = NULL;
};

#endif // HOTKEYHUD_H
