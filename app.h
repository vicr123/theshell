#ifndef APP_H
#define APP_H

#include <QObject>
#include <QIcon>

class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = 0);

    QString name();
    void setName(QString name);

    QIcon icon();
    void setIcon(QIcon icon);

    QString command();
    void setCommand(QString command);

    QString description();
    void setDescription(QString desc);
signals:

public slots:

private:
    QString appname;
    QIcon appicon;
    QString appcommand;
    QString appdesc = "";
};

#endif // APP_H