#ifndef GLOBALFILTER_H
#define GLOBALFILTER_H

#include <QObject>
#include <QApplication>
#include <QUrl>
#include <QSound>
#include <QSettings>
#include "background.h"

class GlobalFilter : public QObject
{
    Q_OBJECT
public:
    explicit GlobalFilter(QApplication *application, QObject *parent = 0);

signals:

public slots:
    void reloadScreens();

private:
    bool eventFilter(QObject *object, QEvent *event);

    QSound* clickSound;
};

#endif // GLOBALFILTER_H
