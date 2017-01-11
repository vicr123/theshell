#ifndef MOUSESCROLLWIDGET_H
#define MOUSESCROLLWIDGET_H

#include <QScrollArea>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QScrollBar>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>

class MouseScrollWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit MouseScrollWidget(QWidget *parent = 0);

    //QSize sizeHint() const;
    void setWidget(QWidget *widget);
signals:

public slots:

private:
    QTimer shiftLeft, shiftRight;
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

    void setEventFilter(QWidget* widget);
    void setEventFilter(QLayout* layout);

    QWidget* vp;

    int scrollSpeed = 0;
};

#endif // MOUSESCROLLWIDGET_H
