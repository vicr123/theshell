#ifndef MOUSESCROLLWIDGET_H
#define MOUSESCROLLWIDGET_H

#include <QScrollArea>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QScrollBar>
#include <QLayout>

class MouseScrollWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit MouseScrollWidget(QWidget *parent = 0);

    QSize sizeHint() const;
    void setWidget(QWidget *widget);
signals:

public slots:

private:
    QTimer shiftLeft, shiftRight;
    void mouseMoveEvent(QMouseEvent* event);
    bool eventFilter(QObject *object, QEvent *event);

    void setEventFilter(QWidget* widget);
    void setEventFilter(QLayout* layout);

    int scrollSpeed = 0;
};

#endif // MOUSESCROLLWIDGET_H
