#ifndef NOTIFICATIONOBJECT_H
#define NOTIFICATIONOBJECT_H

#include <QObject>
#include "notificationpopup.h"

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

    int getId();

signals:
    void parametersUpdated();
    void actionClicked(QString key);
    void closed(NotificationCloseReason reason);

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
};

#endif // NOTIFICATIONOBJECT_H
