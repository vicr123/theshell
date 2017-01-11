#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QStyle>
#include <QApplication>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(QWidget* parent);

    void setShowDisabled(bool showDisabled);
    bool showDisabled();
signals:
    void clicked();
    void dragging(int x, int y);
    void mouseReleased();

private:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    //void paintEvent(QPaintEvent *event);

    bool isShowDisabled = false;
    bool isClicked = false;
    bool didDrag = false;
};

#endif // CLICKABLELABEL_H
