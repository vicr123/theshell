#ifndef APP_H
#define APP_H

#include <QObject>
#include <QIcon>

class App
{

public:
    explicit App();

    QString name();
    void setName(QString name);

    QIcon icon();
    void setIcon(QIcon icon);

    QString command();
    void setCommand(QString command);

    QString description();
    void setDescription(QString desc);

    bool isPinned();
    void setPinned(bool pinned);

    QString desktopEntry();
    void setDesktopEntry(QString entry);
signals:

public slots:

private:
    QString appname;
    QIcon appicon;
    QString appcommand;
    QString appdesc = "";
    QString appfile = "";
    bool pin = false;
};

Q_DECLARE_METATYPE(App)

#endif // APP_H
