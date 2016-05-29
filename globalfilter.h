#ifndef GLOBALFILTER_H
#define GLOBALFILTER_H

#include <QObject>
#include <QApplication>
#include <QUrl>
#include <QSound>
#include <QSettings>

class GlobalFilter : public QObject
{
    Q_OBJECT
public:
    explicit GlobalFilter(QApplication *application, QObject *parent = 0);

signals:

public slots:

private:
    bool eventFilter(QObject *object, QEvent *event);

    QSound* clickSound;
};

#endif // GLOBALFILTER_H
