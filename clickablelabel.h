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
private:
    void mousePressEvent(QMouseEvent* event);
    //void paintEvent(QPaintEvent *event);

    bool isShowDisabled = false;
};

#endif // CLICKABLELABEL_H
