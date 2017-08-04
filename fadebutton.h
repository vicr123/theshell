#ifndef FADEBUTTON_H
#define FADEBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPaintEvent>
#include <tvariantanimation.h>

class FadeButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FadeButton(QWidget *parent = 0);

    bool fade();
    void setFade(bool fade);

    void setFullText(QString fullText);

    void animateIn();
    void animateOut();
signals:

public slots:

private:
    bool f = false;
    QString txt;
};

#endif // FADEBUTTON_H
