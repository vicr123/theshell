#ifndef SWITCH_H
#define SWITCH_H

#include <QWidget>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QStaticText>

class Switch : public QPushButton
{
    Q_OBJECT
public:
    explicit Switch(QWidget *parent = 0);

signals:

public slots:

private:
    void paintEvent(QPaintEvent *event);
    QSize sizeHint();
};

#endif // SWITCH_H
