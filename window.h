#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QIcon>

class WmWindow : public QObject
{
    Q_OBJECT
public:
    explicit WmWindow(QObject *parent = 0);

    QString title();
    void setTitle(QString);

    unsigned long PID();
    void setPID(unsigned long);
    unsigned long WID();
    void setWID(unsigned long);
    QIcon icon();
    void setIcon(QIcon);
signals:

public slots:

private:
    QString winTitle;
    int id;
    unsigned long pid;
    unsigned long wid;
    QIcon ic;
};

#endif // WINDOW_H
