#ifndef NOTIFICATIONOBJECT_H
#define NOTIFICATIONOBJECT_H

#include <QObject>
#include <QMediaPlayer>
#include <QSoundEffect>
#include "notificationpopup.h"
#include "audiomanager.h"

class NotificationPopup;

class NotificationObject : public QObject
{
    Q_OBJECT
public:
    enum NotificationCloseReason : uint {
        Expired = 1,
        Dismissed = 2,
        ProgrammaticallyClosed = 3,
        Undefined = 4
    };

    explicit NotificationObject(QString app_name, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int expire_timeout, QObject *parent = nullptr);
    static int currentId;

    uint getId();

    QString getAppIdentifier();
    QString getAppName();
    QIcon getAppIcon();
    QString getSummary();
    QString getBody();

signals:
    void parametersUpdated();
    void actionClicked(QString key);
    void closed(NotificationObject::NotificationCloseReason reason);

public slots:
    void post();
    void setParameters(QString &app_name, QString &app_icon, QString &summary, QString &body, QStringList &actions, QVariantMap &hints, int expire_timeout);
    void closeDialog();
    void dismiss();

private:
    QString appName, appIcon, summary, body;
    QStringList actions;
    QVariantMap hints;
    int timeout;
    uint id;

    NotificationPopup* dialog;

    QIcon appIc, bigIc;
    QSettings settings;
    QSettings* notificationAppSettings = new QSettings("theSuite", "theShell-notifications", this);
};

#endif // NOTIFICATIONOBJECT_H
