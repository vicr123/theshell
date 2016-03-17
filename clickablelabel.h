#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel
{

    Q_OBJECT

public:
    ClickableLabel(QWidget* parent);

signals:
    void clicked();
private:
    void mousePressEvent(QMouseEvent* event);
};

#endif // CLICKABLELABEL_H
