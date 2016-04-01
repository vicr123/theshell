#ifndef HOVERFRAME_H
#define HOVERFRAME_H

#include <QObject>
#include <QFrame>

class HoverFrame : public QFrame
{
    Q_OBJECT
public:
    explicit HoverFrame(QWidget *parent = 0);

signals:
    void MouseEnter();
    void MouseExit();

public slots:

private:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};

#endif // HOVERFRAME_H
