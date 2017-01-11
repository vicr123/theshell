#ifndef SWITCH_H
#define SWITCH_H

#include <QWidget>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QStaticText>
#include <tvariantanimation.h>

class Switch : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QString OnText READ OnText WRITE setOnText NOTIFY OnTextChanged)
    Q_PROPERTY(QString OffText READ OffText WRITE setOffText NOTIFY OffTextChanged)
    Q_PROPERTY(QIcon OnIcon READ OnIcon WRITE setOnIcon NOTIFY OnIconChanged)
public:
    explicit Switch(QWidget *parent = 0);

    QString OnText();
    QString OffText();
    QIcon OnIcon();
signals:
    void OnTextChanged(QString OnText);
    void OffTextChanged(QString OffText);
    void OnIconChanged(QString OffText);

public slots:
    void setOnText(QString text);
    void setOffText(QString text);
    void setOnIcon(QIcon icon);
private slots:
    void checkChanging(bool checked);

private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QPalette::ColorGroup IsActiveColorRole();
    QSize sizeHint() const;

    QRect innerRect;
    int mouseClickPoint;
    int initialPoint;
    bool mouseMovedLeft = false;

    QString iText = "I";
    QString oText = "O";
    QIcon iIcon;
};

#endif // SWITCH_H
