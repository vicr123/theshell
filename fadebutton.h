#ifndef FADEBUTTON_H
#define FADEBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>

class FadeButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FadeButton(QWidget *parent = 0);

    bool fade();
    void setFade(bool fade);
signals:

public slots:

private:
    bool f = false;
};

#endif // FADEBUTTON_H
