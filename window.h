#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>

class WmWindow : public QObject
{
    Q_OBJECT
public:
    explicit WmWindow(QObject *parent = 0);

    QString title();
    void setTitle(QString);

    int PID();
    void setPID(int);
signals:

public slots:

private:
    QString winTitle;
    int id;
};

#endif // WINDOW_H
