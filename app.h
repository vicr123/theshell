#ifndef APP_H
#define APP_H

#include <QObject>
#include <QIcon>

class App
{

public:
    explicit App();

    QString name() const;
    void setName(QString name);

    QIcon icon() const;
    void setIcon(QIcon icon);

    QString command() const;
    void setCommand(QString command);

    QString description() const;
    void setDescription(QString desc);

    bool isPinned() const;
    void setPinned(bool pinned);

    QString desktopEntry() const;
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
