#ifndef THEWAVEFEEDBACKFRAME_H
#define THEWAVEFEEDBACKFRAME_H

#include <QFrame>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QtMath>

class theWaveFeedbackFrame : public QFrame
{
    Q_OBJECT
public:
    explicit theWaveFeedbackFrame(QWidget *parent = 0);

signals:

public slots:
    void addLevel(qreal level);

private:
    void paintEvent(QPaintEvent *event);

    QList<qreal> levels;
    bool indeterminate = false;
    int indeterminateStage = 0;
    QTimer indeterminateTimer;
};

#endif // THEWAVEFEEDBACKFRAME_H
